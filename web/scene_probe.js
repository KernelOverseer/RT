/*
** per-scene probe (one process per scene):
**   node scene_probe.js <scene.xml> [height]
** renders a full single pass and reports coverage + timing.
*/

const fs = require('fs');
const path = require('path');
const RTModule = require(path.join(__dirname, 'dist', 'rt.js'));

const scene = process.argv[2];
const height = parseInt(process.argv[3] || '240', 10);

async function main() {
  const mod = await Promise.resolve(RTModule({}));
  mod._rt_web_init();
  const xml = fs.readFileSync(path.join(__dirname, 'dist', 'scenes', scene), 'utf8');
  mod.FS.writeFile('/s.xml', xml);
  const p = mod.allocateUTF8('/s.xml');
  if (mod._rt_web_load_scene(p) !== 0) {
    console.log(`${scene} PARSE_FAIL`);
    process.exit(2);
  }
  mod._free(p);
  mod._rt_web_set_resolution(height);
  const t0 = Date.now();
  mod._rt_web_begin_pass(2, 2, 0);
  mod._rt_web_render_band(0, 1);
  const ms = Date.now() - t0;
  const ptr = mod._rt_web_pixels();
  const len = mod._rt_web_pixels_len();
  const buf = new Uint32Array(mod.HEAPU8.buffer, ptr, len / 4);
  let nonZero = 0;
  let colored = 0;
  for (let i = 0; i < buf.length; i++) {
    const px = buf[i] & 0xffffff;
    if (px !== 0) nonZero++;
    const r = (px >> 16) & 0xff, g = (px >> 8) & 0xff, b = px & 0xff;
    if (Math.max(r, g, b) - Math.min(r, g, b) > 24) colored++;
  }
  console.log(`${scene} nonZero=${nonZero}/${buf.length} colored=${colored} ${ms}ms`);
  process.exit(nonZero > 0 ? 0 : 1);
}
main().catch((e) => { console.error(scene, 'ERROR', e.message); process.exit(3); });
setTimeout(() => { console.log(`${scene} TIMEOUT`); process.exit(4); }, 120000).unref();
