/*
** RT portfolio driver: worker pool, progressive renderer, camera controls.
**
** The camera model replicates the native GUI (key_managing.c): a yaw/pitch
** pair generates a look offset of radius 5 around the position, W/S moves
** along the view direction, click casts a ray to re-aim and set the depth
** of field focus. Rendering is progressive, coarse pass first, like the
** original frame loop, and restarts whenever the camera moves.
**
** Engine pixels are 0x00RRGGBB; the compositor ORs in the alpha byte for
** canvas ImageData (0xAABBGGRR in little-endian Uint32 view).
*/

'use strict';

/* ------------------------------------------------------------------ */
/* state                                                               */
/* ------------------------------------------------------------------ */

const WORKER_COUNT = Math.min(6, Math.max(1, (navigator.hardwareConcurrency || 4) - 1));

const canvas = document.getElementById('view');
const ctx = canvas.getContext('2d', { alpha: false });

const state = {
  scene: null,
  sceneInfo: null,
  camera: null,
  angles: null,
  dofFocus: 10,
  options: { antiAliasing: 1, ambiant: 1, diffuse: 1, specular: 1, refraction: 1, reflection: 1, softShadows: 1, depthOfField: 0 },
  quality: 'normal',
  resolution: 480,
  filter: 0,
  workers: [],
  readyCount: 0,
  epoch: 0,
  ladder: [],
  currentPass: null,
  inFlight: 0,
  pendingDispatch: false,
  width: 0,
  height: 0,
  image: null,
  imageData: null,
  lastFull: null,
  blitQueued: false,
  moving: false,
  idleTimer: 0,
};

const QUALITY_PRESETS = {
  draft: { lightSamples: 1, reflectionDepth: 2, refractionDepth: 2, dofSamples: 2 },
  normal: { lightSamples: 6, reflectionDepth: 3, refractionDepth: 3, dofSamples: 5 },
  high: { lightSamples: 20, reflectionDepth: 3, refractionDepth: 3, dofSamples: 8 },
};

const FINAL_LADDER = [
  { pixelSize: 8, offset: 8 },
  { pixelSize: 4, offset: 4 },
  { pixelSize: 2, offset: 2 },
  { pixelSize: 1, offset: 0 },
];

const MOVE_SPEED = 18;          // world units / s (native: 5 per keypress)
const ROT_SPEED = Math.PI / 2;  // rad / s (native: PI/36 per keypress)
const LOOK_RADIUS = 5;          // native constant from ft_move_cam
const keys = {};

/* ------------------------------------------------------------------ */
/* workers                                                             */
/* ------------------------------------------------------------------ */

let requestSeq = 0;
const requestWaiters = new Map();

function post(worker, msg) {
  msg.id = ++requestSeq;
  return new Promise((resolve) => {
    requestWaiters.set(msg.id, resolve);
    worker.postMessage(msg);
  });
}

function broadcast(msg) {
  return Promise.all(state.workers.map((w) => post(w, msg)));
}

function onWorkerMessage(msg) {
  if (msg.type === 'ready') {
    state.readyCount++;
    if (state.readyCount === state.workers.length) onAllWorkersReady();
    return;
  }
  if (msg.type === 'error') {
    setStatus('engine error: ' + msg.message, true);
    return;
  }
  if (msg.type === 'band') {
    onBand(msg);
    return;
  }
  const waiter = requestWaiters.get(msg.id);
  if (waiter) {
    requestWaiters.delete(msg.id);
    waiter(msg);
  }
}

async function onAllWorkersReady() {
  document.getElementById('worker-count').textContent =
    state.workers.length + (state.workers.length > 1 ? ' workers' : ' worker');
  await loadScene(SCENES.find((s) => s.file === DEFAULT_SCENE) || SCENES[0]);
  document.getElementById('loading-overlay').classList.add('hidden');
}

/* ------------------------------------------------------------------ */
/* scene loading                                                       */
/* ------------------------------------------------------------------ */

async function loadScene(entry) {
  state.scene = entry;
  setStatus('loading ' + entry.name + '…');
  const xml = await (await fetch('scenes/' + entry.file)).text();
  const replies = await broadcast({ type: 'load', xml, height: state.resolution });
  const first = replies.find((r) => r.type === 'loaded');
  if (!first) {
    setStatus('scene failed to load', true);
    return;
  }
  state.sceneInfo = {
    width: first.width,
    height: first.height,
    aa: first.aa,
    dof: first.dof,
    lightSamples: first.lightSamples,
    camera: first.camera,
  };
  QUALITY_PRESETS.high = {
    lightSamples: Math.max(first.lightSamples, 8),
    reflectionDepth: 3,
    refractionDepth: 3,
    dofSamples: Math.max(first.dof, 5),
  };
  resetCamera();
  allocateBuffers();
  document.getElementById('scene-description').textContent = entry.description;
  scheduleRender(false);
}

function resetCamera() {
  const c = state.sceneInfo.camera;
  state.camera = { pos: [c[0], c[1], c[2]], look: [c[3], c[4], c[5]], fov: c[6] };
  state.angles = null;
  state.dofFocus = 10;
}

function allocateBuffers() {
  state.width = state.sceneInfo.width;
  state.height = state.sceneInfo.height;
  canvas.width = state.width;
  canvas.height = state.height;
  state.image = new Uint32Array(state.width * state.height).fill(0xff000000);
  state.imageData = ctx.createImageData(state.width, state.height);
  state.lastFull = null;
}

/* ------------------------------------------------------------------ */
/* render scheduling                                                   */
/* ------------------------------------------------------------------ */

function optionsArray() {
  const o = state.options;
  const interactive = state.moving;
  return [
    interactive ? 0 : o.antiAliasing,
    o.ambiant, o.diffuse, o.specular, o.refraction, o.reflection,
    o.softShadows, o.depthOfField,
  ];
}

function qualityArray() {
  const preset = state.moving ? QUALITY_PRESETS.draft
    : (QUALITY_PRESETS[state.quality] || QUALITY_PRESETS.normal);
  return [preset.lightSamples, preset.reflectionDepth, preset.refractionDepth, preset.dofSamples];
}

function cameraArray() {
  const c = state.camera;
  return [c.pos[0], c.pos[1], c.pos[2], c.look[0], c.look[1], c.look[2], c.fov];
}

function scheduleRender(interactive) {
  if (!state.sceneInfo) return;
  state.epoch++;
  const aa = state.options.antiAliasing ? Math.min(state.sceneInfo.aa || 4, 8) : 0;
  if (interactive) {
    state.ladder = FINAL_LADDER.slice(0, 3).map((p) => ({ ...p, aa: 0 }));
    state.moving = true;
    clearTimeout(state.idleTimer);
    state.idleTimer = setTimeout(() => {
      state.moving = false;
      scheduleRender(false);
    }, 350);
  } else {
    state.moving = false;
    clearTimeout(state.idleTimer);
    state.ladder = FINAL_LADDER.map((p, i) => ({ ...p, aa: i === FINAL_LADDER.length - 1 ? aa : 0 }));
  }
  state.pendingDispatch = true;
  tryDispatch();
}

function tryDispatch() {
  // wait for in-flight bands so camera motion does not starve the display
  if (!state.pendingDispatch || state.inFlight > 0 || !state.ladder.length) return;
  state.pendingDispatch = false;
  nextPass();
}

function nextPass() {
  if (!state.ladder.length) {
    onFinishLadder();
    return;
  }
  const pass = state.ladder.shift();
  state.currentPass = pass;
  setStatus(passLabel(pass));
  state.inFlight = state.workers.length;
  state.workers.forEach((worker, i) => {
    const msg = {
      type: pass.stereo ? 'stereo' : 'render',
      id: ++requestSeq,
      epoch: state.epoch,
      pass,
      camera: cameraArray(),
      options: optionsArray(),
      quality: qualityArray(),
      dofFocus: state.dofFocus,
      filter: state.filter,
      bandIndex: i,
      bandCount: state.workers.length,
    };
    requestWaiters.set(msg.id, () => {});
    worker.postMessage(msg);
  });
  updateProgress(0);
}

function passLabel(pass) {
  if (pass.stereo) return 'anaglyph stereo';
  if (pass.aa) return 'anti-aliasing ×' + (pass.aa + 1);
  if (pass.pixelSize === 1) return 'full pass';
  return 'preview 1/' + pass.pixelSize;
}

function onBand(msg) {
  state.inFlight = Math.max(0, state.inFlight - 1);
  if (msg.epoch !== state.epoch) {
    tryDispatch(); // superseded pass, maybe a newer one is waiting
    return;
  }
  const stripe = new Uint32Array(msg.pixels);
  const sw = msg.x1 - msg.x0;
  for (let y = 0; y < msg.height; y++) {
    const src = y * sw;
    const dst = y * msg.width + msg.x0;
    for (let x = 0; x < sw; x++)
      state.image[dst + x] = 0xff000000 | stripe[src + x];
  }
  updateProgress(1 - state.inFlight / state.workers.length);
  queueBlit(msg.pass.pixelSize > 1);
  if (state.inFlight === 0) {
    if (state.ladder.length) nextPass();
    else if (state.pendingDispatch) tryDispatch();
    else onFinishLadder();
  }
}

function updateProgress(fraction) {
  document.getElementById('progress-fill').style.width = Math.round(fraction * 100) + '%';
}

function dilate() {
  // fill the holes of a coarse pass for display: copy left/above neighbor
  const w = state.width, h = state.height, img = state.image;
  for (let y = 1; y < h; y++) {
    for (let x = 1; x < w; x++) {
      const i = y * w + x;
      if ((img[i] & 0xffffff) === 0)
        img[i] = img[i - 1] !== 0xff000000 ? img[i - 1] : (img[i - w] || img[i]);
    }
  }
}

function queueBlit(coarse) {
  if (coarse) dilate();
  if (state.blitQueued) return;
  state.blitQueued = true;
  requestAnimationFrame(() => {
    state.blitQueued = false;
    new Uint8Array(state.imageData.data.buffer).set(
      new Uint8Array(state.image.buffer, state.image.byteOffset, state.image.length * 4));
    ctx.putImageData(state.imageData, 0, 0);
  });
}

function onFinishLadder() {
  state.lastFull = state.image.slice().buffer;
  setStatus('done — drag to look around, WASD to move');
  updateProgress(1);
  const effect = parseInt(document.getElementById('effect-select').value, 10);
  if (effect) applyEffect(effect);
}

/* ------------------------------------------------------------------ */
/* camera controls (mirrors key_managing.c / ft_change_lookat)          */
/* ------------------------------------------------------------------ */

function deriveAngles() {
  const d = [
    state.camera.look[0] - state.camera.pos[0],
    state.camera.look[1] - state.camera.pos[1],
    state.camera.look[2] - state.camera.pos[2],
  ];
  const len = Math.hypot(d[0], d[1], d[2]) || 1;
  state.angles = {
    yaw: Math.atan2(d[2], d[0]),
    pitch: Math.asin(Math.max(-1, Math.min(1, d[1] / len))),
  };
}

function applyAngles() {
  const a = state.angles;
  state.camera.look = [
    state.camera.pos[0] + LOOK_RADIUS * Math.cos(a.yaw) * Math.cos(a.pitch),
    state.camera.pos[1] + LOOK_RADIUS * Math.sin(a.pitch),
    state.camera.pos[2] + LOOK_RADIUS * Math.sin(a.yaw) * Math.cos(a.pitch),
  ];
}

function forwardVector() {
  const f = [
    state.camera.look[0] - state.camera.pos[0],
    state.camera.look[1] - state.camera.pos[1],
    state.camera.look[2] - state.camera.pos[2],
  ];
  const len = Math.hypot(f[0], f[1], f[2]) || 1;
  return [f[0] / len, f[1] / len, f[2] / len];
}

function moveCamera(dt) {
  if (state.angles === null) deriveAngles();
  let changed = false;

  const yawRate = (keys.a ? -1 : 0) + (keys.d ? 1 : 0)
    + (keys.arrowleft ? -1 : 0) + (keys.arrowright ? 1 : 0);
  const pitchRate = (keys.arrowup ? 1 : 0) + (keys.arrowdown ? -1 : 0);
  if (yawRate || pitchRate) {
    state.angles.yaw += yawRate * ROT_SPEED * dt;
    state.angles.pitch = Math.max(-1.5, Math.min(1.5,
      state.angles.pitch + pitchRate * ROT_SPEED * dt));
    changed = true;
  }

  const f = forwardVector();
  if (keys.w || keys.s) {
    const step = (keys.w ? 1 : -1) * MOVE_SPEED * dt;
    for (let i = 0; i < 3; i++) state.camera.pos[i] += f[i] * step;
    changed = true;
  }
  if (keys.q || keys.e) {
    const rl = Math.hypot(f[0], f[2]) || 1;
    const step = (keys.e ? 1 : -1) * MOVE_SPEED * dt;
    state.camera.pos[0] += (f[2] / rl) * step;
    state.camera.pos[2] += (-f[0] / rl) * step;
    changed = true;
  }
  if (keys.r) { state.dofFocus += 12 * dt; changed = true; }
  if (keys.t) { state.dofFocus = Math.max(1, state.dofFocus - 12 * dt); changed = true; }

  if (changed) applyAngles();
  return changed;
}

let lastFrame = performance.now();
function inputLoop(now) {
  const dt = Math.min(0.1, (now - lastFrame) / 1000);
  lastFrame = now;
  if (state.sceneInfo && Object.values(keys).some(Boolean) && moveCamera(dt))
    scheduleRender(true);
  requestAnimationFrame(inputLoop);
}

/* mouse look + click-to-focus */

let dragging = false;
let dragMoved = 0;
let lastX = 0;
let lastY = 0;

function dragLook(dx, dy) {
  if (state.angles === null) deriveAngles();
  state.angles.yaw += dx * 0.004;
  state.angles.pitch = Math.max(-1.5, Math.min(1.5, state.angles.pitch - dy * 0.004));
  applyAngles();
  scheduleRender(true);
}

canvas.addEventListener('mousedown', (e) => {
  dragging = true;
  dragMoved = 0;
  lastX = e.clientX;
  lastY = e.clientY;
});

window.addEventListener('mousemove', (e) => {
  if (!dragging) return;
  const dx = e.clientX - lastX;
  const dy = e.clientY - lastY;
  lastX = e.clientX;
  lastY = e.clientY;
  dragMoved += Math.abs(dx) + Math.abs(dy);
  if (dragMoved >= 4 && state.sceneInfo) dragLook(dx, dy);
});

window.addEventListener('mouseup', (e) => {
  if (!dragging) return;
  dragging = false;
  if (dragMoved < 4 && state.sceneInfo && e.target === canvas) clickFocus(e);
});

canvas.addEventListener('touchstart', (e) => {
  if (!e.touches.length) return;
  dragging = true;
  dragMoved = 0;
  lastX = e.touches[0].clientX;
  lastY = e.touches[0].clientY;
}, { passive: true });

canvas.addEventListener('touchmove', (e) => {
  if (!dragging || !e.touches.length || !state.sceneInfo) return;
  const t = e.touches[0];
  const dx = t.clientX - lastX;
  const dy = t.clientY - lastY;
  lastX = t.clientX;
  lastY = t.clientY;
  dragMoved += Math.abs(dx) + Math.abs(dy);
  dragLook(dx * 1.2, dy * 1.2);
  e.preventDefault();
}, { passive: false });

canvas.addEventListener('touchend', () => { dragging = false; });

async function clickFocus(e) {
  const rect = canvas.getBoundingClientRect();
  const x = Math.floor((e.clientX - rect.left) / rect.width * state.width);
  const y = Math.floor((e.clientY - rect.top) / rect.height * state.height);
  const reply = await post(state.workers[0], {
    type: 'raycast', x, y, camera: cameraArray(),
  });
  if (!reply || reply.distance < 0) {
    setStatus('nothing to focus on there');
    return;
  }
  state.camera.look = reply.camera.slice(3, 6);
  state.dofFocus = reply.dofFocus;
  state.angles = null;
  setStatus('focused at ' + reply.distance.toFixed(1) + ' units');
  scheduleRender(false);
}

/* keyboard */

const KEY_MAP = {
  KeyW: 'w', KeyS: 's', KeyA: 'a', KeyD: 'd', KeyQ: 'q', KeyE: 'e',
  KeyR: 'r', KeyT: 't',
  ArrowUp: 'arrowup', ArrowDown: 'arrowdown',
  ArrowLeft: 'arrowleft', ArrowRight: 'arrowright',
};

window.addEventListener('keydown', (e) => {
  const k = KEY_MAP[e.code];
  if (!k) return;
  keys[k] = true;
  e.preventDefault();
});

window.addEventListener('keyup', (e) => {
  const k = KEY_MAP[e.code];
  if (k) keys[k] = false;
});

/* ------------------------------------------------------------------ */
/* effects, stereo, save                                               */
/* ------------------------------------------------------------------ */

async function applyEffect(effect) {
  if (!state.lastFull) return;
  setStatus('applying filter…');
  const reply = await post(state.workers[0], { type: 'effect', effect, pixels: state.lastFull });
  if (!state.image || reply.width !== state.width || reply.height !== state.height) return;
  const px = new Uint32Array(reply.pixels);
  for (let i = 0; i < state.image.length; i++) state.image[i] = 0xff000000 | px[i];
  queueBlit(false);
  setStatus('done — post filter applied');
}

function runStereo() {
  if (!state.sceneInfo) return;
  state.epoch++;
  state.pendingDispatch = false;
  state.ladder = [{ pixelSize: 1, offset: 0, aa: 0, stereo: true }];
  nextPass();
}

document.getElementById('stereo-button').addEventListener('click', runStereo);

document.getElementById('save-button').addEventListener('click', async () => {
  if (!state.lastFull) return;
  setStatus('saving…');
  const reply = await post(state.workers[0], { type: 'save', pixels: state.lastFull });
  const blob = new Blob([reply.bytes], { type: 'image/bmp' });
  const a = document.createElement('a');
  a.href = URL.createObjectURL(blob);
  a.download = (state.scene ? state.scene.file.replace('.xml', '') : 'rt') + '.bmp';
  a.click();
  URL.revokeObjectURL(a.href);
  setStatus('saved ' + a.download);
});

/* ------------------------------------------------------------------ */
/* UI                                                                  */
/* ------------------------------------------------------------------ */

function setStatus(text, isError) {
  const el = document.getElementById('status');
  el.textContent = text;
  el.classList.toggle('error', !!isError);
}

function buildUi() {
  const sceneSelect = document.getElementById('scene-select');
  for (const s of SCENES) {
    const opt = document.createElement('option');
    opt.value = s.file;
    opt.textContent = s.name;
    sceneSelect.appendChild(opt);
  }
  sceneSelect.value = DEFAULT_SCENE;
  sceneSelect.addEventListener('change', () => {
    const entry = SCENES.find((s) => s.file === sceneSelect.value);
    if (entry) loadScene(entry);
  });

  document.getElementById('quality-select').addEventListener('change', (e) => {
    state.quality = e.target.value;
    scheduleRender(false);
  });

  document.getElementById('resolution-select').addEventListener('change', (e) => {
    state.resolution = parseInt(e.target.value, 10);
    if (state.scene) loadScene(state.scene);
  });

  document.getElementById('filter-select').addEventListener('change', (e) => {
    state.filter = parseInt(e.target.value, 10);
    scheduleRender(false);
  });

  document.getElementById('effect-select').addEventListener('change', (e) => {
    applyEffect(parseInt(e.target.value, 10));
  });

  const TOGGLES = [
    ['antiAliasing', 'Anti-aliasing'],
    ['ambiant', 'Ambient'],
    ['diffuse', 'Diffuse'],
    ['specular', 'Specular'],
    ['refraction', 'Refraction'],
    ['reflection', 'Reflection'],
    ['softShadows', 'Soft shadows'],
    ['depthOfField', 'Depth of field'],
  ];
  const toggleBox = document.getElementById('toggles');
  for (const [key, label] of TOGGLES) {
    const id = 'toggle-' + key;
    const lab = document.createElement('label');
    const input = document.createElement('input');
    input.type = 'checkbox';
    input.id = id;
    input.checked = !!state.options[key];
    input.addEventListener('change', () => {
      state.options[key] = input.checked ? 1 : 0;
      scheduleRender(false);
    });
    lab.htmlFor = id;
    lab.textContent = label;
    lab.prepend(input);
    toggleBox.appendChild(lab);
  }

  document.getElementById('reset-button').addEventListener('click', () => {
    resetCamera();
    scheduleRender(false);
  });
}

buildUi();
state.workers = Array.from({ length: WORKER_COUNT }, () => {
  const worker = new Worker('worker.js');
  worker.onmessage = (ev) => onWorkerMessage(ev.data);
  return worker;
});
requestAnimationFrame(inputLoop);
