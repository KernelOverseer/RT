/*
** node smoke test for the wasm build (also validates the scene catalog):
**   node smoke_test.js
** loads every scene in dist/scenes, renders one coarse pass and reports
** scene dimensions, render time and how many pixels got colored.
*/

const fs = require('fs');
const path = require('path');
const RTModule = require(path.join(__dirname, 'dist', 'rt.js'));

const RESOLUTION = 360; // low res keeps the test fast

function renderScene(mod, xml) {
  mod._rt_web_dispose();
  mod.FS.writeFile('/scene.xml', xml);
  const pathPtr = mod.allocateUTF8('/scene.xml');
  const status = mod._rt_web_load_scene(pathPtr);
  mod._free(pathPtr);
  if (status !== 0)
    return { error: 'parse failed' };
  mod._rt_web_set_resolution(RESOLUTION);
  const w = mod._rt_web_scene_width();
  const h = mod._rt_web_scene_height();
  const t0 = process.hrtime.bigint();
  mod._rt_web_begin_pass(2, 2, 0);
  mod._rt_web_render_band(0, 1);
  const ms = Number(process.hrtime.bigint() - t0) / 1e6;
  const ptr = mod._rt_web_pixels();
  const len = mod._rt_web_pixels_len();
  const buf = new Uint32Array(mod.HEAPU8.buffer, ptr, len / 4);
  let nonZero = 0;
  for (let i = 0; i < buf.length; i++)
    if (buf[i] !== 0) nonZero++;
  return { w, h, ms: ms.toFixed(0), nonZero };
}

async function main() {
  const mod = await Promise.resolve(RTModule({
    print: (t) => process.stdout.write(String(t) + '\n'),
  }));
  mod._rt_web_init();

  const scenesDir = path.join(__dirname, 'dist', 'scenes');
  const scenes = fs.readdirSync(scenesDir).filter((f) => f.endsWith('.xml'));
  scenes.sort();

  let good = 0;
  for (const scene of scenes) {
    const xml = fs.readFileSync(path.join(scenesDir, scene), 'utf8');
    let result;
    try {
      result = renderScene(mod, xml);
    } catch (e) {
      result = { error: e.message };
    }
    const status = result.error ? `FAIL (${result.error})`
      : result.nonZero > 0 ? 'ok' : 'FAIL (all black)';
    if (!result.error && result.nonZero > 0) good++;
    console.log(
      `${scene.padEnd(28)} ${status.padEnd(22)}`,
      result.w ? `${result.w}x${result.h}  ${result.nonZero}/${result.w * result.h} px  ${result.ms}ms` : ''
    );
  }
  console.log(`\n${good}/${scenes.length} scenes render`);
  process.exit(good === 0 ? 1 : 0);
}

main().catch((e) => { console.error(e); process.exit(1); });
