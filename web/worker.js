/*
** RT render worker: one WASM engine instance per worker.
** Each worker renders one vertical band of the image (the same band split
** the original pthread build used) and posts its stripe back as a
** transferable buffer.
**
** main.js spawns this file with ?v=<build> for cache busting; reuse it for
** the engine assets so a fresh worker never loads a stale rt.js/rt.wasm.
*/

const SELF_VERSION = new URL(self.location.href).searchParams.get('v') || 'dev';

importScripts('rt.js?v=' + SELF_VERSION);

let mod = null;
let booted = false;
const pending = [];

function handle(msg) {
  try {
    switch (msg.type) {
      case 'load': {
        mod._rt_web_dispose();
        mod.FS.writeFile('/scene.xml', msg.xml);
        const path = mod.allocateUTF8('/scene.xml');
        const status = mod._rt_web_load_scene(path);
        mod._free(path);
        if (status !== 0) {
          self.postMessage({ type: 'load-failed', id: msg.id });
          return;
        }
        if (msg.height) mod._rt_web_set_resolution(msg.height);
        const camPtr = mod._malloc(8 * 7);
        mod._rt_web_get_camera(camPtr);
        const camera = Array.from(new Float64Array(mod.HEAPU8.buffer, camPtr, 7));
        mod._free(camPtr);
        self.postMessage({
          type: 'loaded',
          id: msg.id,
          width: mod._rt_web_scene_width(),
          height: mod._rt_web_scene_height(),
          aa: mod._rt_web_scene_aa(),
          dof: mod._rt_web_scene_dof(),
          lightSamples: mod._rt_web_scene_light_samples(),
          reflectionDepth: mod._rt_web_scene_reflection_depth(),
          refractionDepth: mod._rt_web_scene_refraction_depth(),
          aperture: mod._rt_web_get_dof_aperture(),
          camera,
        });
        return;
      }
      case 'render':
      case 'stereo': {
        const c = msg.camera;
        mod._rt_web_set_camera(c[0], c[1], c[2], c[3], c[4], c[5], c[6]);
        const o = msg.options;
        mod._rt_web_set_options(o[0], o[1], o[2], o[3], o[4], o[5], o[6], o[7]);
        const q = msg.quality;
        mod._rt_web_set_quality(q[0], q[1], q[2], q[3]);
        if (typeof msg.aperture === 'number')
          mod._rt_web_set_dof_aperture(msg.aperture);
        mod._rt_web_set_dof_focus(msg.dofFocus);
        mod._rt_web_set_filter(msg.filter || 0);
        mod._rt_web_begin_pass(msg.pass.pixelSize, msg.pass.offset, msg.pass.aa);
        if (msg.type === 'render')
          mod._rt_web_render_band(msg.bandIndex, msg.bandCount);
        else
          mod._rt_web_stereo_band(msg.bandIndex, msg.bandCount);
        const w = mod._rt_web_scene_width();
        const h = mod._rt_web_scene_height();
        const bandWidth = Math.floor(w / msg.bandCount);
        const x0 = bandWidth * msg.bandIndex;
        const x1 = msg.bandIndex === msg.bandCount - 1 ? w : bandWidth * (msg.bandIndex + 1);
        const pixels = new Uint32Array(mod.HEAPU8.buffer, mod._rt_web_pixels(), w * h);
        const stripe = new Uint32Array((x1 - x0) * h);
        for (let y = 0; y < h; y++)
          stripe.set(pixels.subarray(y * w + x0, y * w + x1), y * (x1 - x0));
        self.postMessage({
          type: 'band',
          id: msg.id,
          epoch: msg.epoch,
          pass: msg.pass,
          stereo: msg.type === 'stereo',
          x0, x1,
          width: w,
          height: h,
          pixels: stripe.buffer,
        }, [stripe.buffer]);
        return;
      }
      case 'bench': {
        const c = msg.camera;
        mod._rt_web_set_camera(c[0], c[1], c[2], c[3], c[4], c[5], c[6]);
        const o = msg.options;
        mod._rt_web_set_options(o[0], o[1], o[2], o[3], o[4], o[5], o[6], o[7]);
        const q = msg.quality;
        mod._rt_web_set_quality(q[0], q[1], q[2], q[3]);
        mod._rt_web_set_dof_aperture(msg.aperture || 0.5);
        mod._rt_web_set_dof_focus(msg.dofFocus || 30);
        mod._rt_web_set_filter(msg.filter || 0);
        mod._rt_web_begin_pass(1, 0, msg.pass.aa);

        const w = mod._rt_web_scene_width();
        const h = mod._rt_web_scene_height();
        const tileW = Math.floor(w / msg.tilesX);
        const tileH = Math.floor(h / msg.tilesY);
        const x0 = (msg.tile % msg.tilesX) * tileW;
        const y0 = Math.floor(msg.tile / msg.tilesX) * tileH;
        const x1 = (msg.tile % msg.tilesX === msg.tilesX - 1) ? w : x0 + tileW;
        const y1 = (Math.floor(msg.tile / msg.tilesX) === msg.tilesY - 1) ? h : y0 + tileH;

        // stream one row at a time so the page shows pixels as they are
        // generated, like the native bench pushing scanlines to the window;
        // only the C render time is charged to the tile's timing
        let renderMs = 0;
        for (let row = 0; row < y1 - y0; row++) {
          const t0 = performance.now();
          mod._rt_web_render_tile_rows(msg.tile, msg.tilesX, msg.tilesY, row, 1);
          renderMs += performance.now() - t0;
          const pixels = new Uint32Array(mod.HEAPU8.buffer, mod._rt_web_pixels(), w * h);
          const line = new Uint32Array(x1 - x0);
          line.set(pixels.subarray((y0 + row) * w + x0, (y0 + row) * w + x1));
          self.postMessage({
            type: 'bench-tile',
            id: msg.id,
            tile: msg.tile,
            x0, y0, x1, y1,
            rowY: y0 + row,
            done: row === y1 - y0 - 1,
            ms: renderMs,
            pixels: line.buffer,
          }, [line.buffer]);
        }
        return;
      }
      case 'raycast': {
        const c = msg.camera;
        mod._rt_web_set_camera(c[0], c[1], c[2], c[3], c[4], c[5], c[6]);
        const distance = mod._rt_web_lookat(msg.x, msg.y);
        const camPtr = mod._malloc(8 * 7);
        mod._rt_web_get_camera(camPtr);
        const camera = Array.from(new Float64Array(mod.HEAPU8.buffer, camPtr, 7));
        mod._free(camPtr);
        self.postMessage({
          type: 'raycast',
          id: msg.id,
          distance,
          dofFocus: mod._rt_web_get_dof_focus(),
          camera,
        });
        return;
      }
      case 'effect': {
        const ptr = mod._malloc(msg.pixels.byteLength);
        new Uint8Array(mod.HEAPU8.buffer, ptr, msg.pixels.byteLength)
          .set(new Uint8Array(msg.pixels));
        mod._rt_web_upload_pixels(ptr, msg.pixels.byteLength);
        mod._free(ptr);
        mod._rt_web_set_effect(msg.effect);
        mod._rt_web_apply_effect();
        const src = mod._rt_web_pixels();
        const len = mod._rt_web_pixels_len();
        self.postMessage({
          type: 'effect-done',
          id: msg.id,
          width: mod._rt_web_scene_width(),
          height: mod._rt_web_scene_height(),
          pixels: mod.HEAPU8.buffer.slice(src, src + len),
        }, []);
        return;
      }
      case 'save': {
        const ptr = mod._malloc(msg.pixels.byteLength);
        new Uint8Array(mod.HEAPU8.buffer, ptr, msg.pixels.byteLength)
          .set(new Uint8Array(msg.pixels));
        mod._rt_web_upload_pixels(ptr, msg.pixels.byteLength);
        mod._free(ptr);
        const path = mod.allocateUTF8('/shot.bmp');
        mod._rt_web_save_bmp(path);
        mod._free(path);
        const bytes = mod.FS.readFile('/shot.bmp');
        mod.FS.unlink('/shot.bmp');
        self.postMessage({ type: 'saved', id: msg.id, bytes: bytes.buffer }, [bytes.buffer]);
        return;
      }
    }
  } catch (e) {
    self.postMessage({ type: 'error', message: String((e && e.message) || e) });
  }
}

Promise.resolve(RTModule({
  locateFile: (path) => path + '?v=' + SELF_VERSION,
})).then((instance) => {
  mod = instance;
  mod._rt_web_init();
  booted = true;
  self.postMessage({ type: 'ready' });
  for (const msg of pending.splice(0)) handle(msg);
}).catch((e) => {
  self.postMessage({ type: 'error', message: String(e) });
});

self.onmessage = (event) => {
  if (!booted) pending.push(event.data);
  else handle(event.data);
};
