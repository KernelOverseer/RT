/*
** node smoke test for the wasm build (also validates the scene catalog):
**   node smoke_test.js            -> fast CI subset (~30s locally)
**   node smoke_test.js all        -> every bundled scene (~5 min locally)
**   node smoke_test.js a.xml b... -> explicit scenes
** loads each scene, renders one coarse pass and reports scene dimensions,
** render time and how many pixels got colored.
**
** FAST_SCENES keeps one scene per engine path, all comfortably under 10s:
** the heavy ones (bench_cat, forest, mirror_room, refraction_gallery,
** ring_study, cubics, bench_mandelbulb) only run with `all`.
*/

const FAST_SCENES = [
  'rgb_studio.xml',            // multi-light mixing, glass + mirror spheres
  'complex_objects.xml',       // hyperboloids, paraboloids, holo-cubes, pills
  'perturbation.xml',          // procedural textures + bump mapping
  'planet_rise.xml',           // parallel light, torus, pastel/marble maps
  'parallel_light.xml',        // directional light + reflective floor
  'mandelbulb.xml',            // fractal ray marching
  'reflection_tranparency.xml',// refraction/transparency depth
  'depth_of_field.xml',        // DOF sampling
  'demo.xml',                  // the canonical multi-primitive scene
];

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
  const available = fs.readdirSync(scenesDir).filter((f) => f.endsWith('.xml')).sort();
  const args = process.argv.slice(2);
  const scenes = args.includes('all') ? available
    : args.length ? args
    : FAST_SCENES;
  const missing = scenes.filter((s) => !available.includes(s));
  if (missing.length) {
    console.error('not in dist/scenes:', missing.join(', '));
    process.exit(1);
  }

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
  console.log(`\n${good}/${scenes.length} scenes render` +
    (scenes.length === available.length ? '' : ` (${available.length} total, run 'node smoke_test.js all' for every scene)`));
  process.exit(good === scenes.length ? 0 : 1);
}

main().catch((e) => { console.error(e); process.exit(1); });
