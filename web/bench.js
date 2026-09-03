/*
** RTBench web driver: tile-based benchmark on the worker pool.
**
** Mirrors the native RTBench design — a 32x20 tile grid, tiles handed out
** dynamically as workers free up (the web equivalent of its
** next_cluster_index mutex loop) — with a corrected score: wall time is
** measured from the first tile dispatch to the last tile received (worker
** spawning is outside the clock), and points are normalized by rendered
** samples so resolutions and AA levels compare fairly.
*/

'use strict';

const TILES_X = 32;
const TILES_Y = 20;
const MAX_WORKERS = 32;
const DEFAULT_WORKERS = Math.min(8, Math.max(1, (navigator.hardwareConcurrency || 4) - 1));

const BENCH_SCENES = [
  {
    file: 'bench_mandelbulb.xml',
    name: 'Mandelbulb (RTBench)',
    description: 'RTBench\'s own scene: a 50-iteration, power-5 ray-marched Mandelbulb. Distance-field marching dominates.',
  },
  {
    file: 'refraction_gallery.xml',
    name: 'Refraction gallery',
    description: 'Five glass spheres, refraction depth 8 — recursion and quartic-free intersection paths.',
  },
  {
    file: 'forest.xml',
    name: 'Forest',
    description: 'Hundreds of primitives — object-list traversal and shadow rays dominate.',
  },
  {
    file: 'mirror_room.xml',
    name: 'Mirror room',
    description: 'Reflection depth 5 in a hall of mirrors — recursive reflection cost.',
  },
  {
    file: 'bench_cat.xml',
    name: 'The cat (extreme)',
    description: 'RTBench\'s 282-triangle refractive mesh in a walled room, reflection depth 8. Heavy — expect a minute or so.',
  },
];

const canvas = document.getElementById('view');
const ctx = canvas.getContext('2d', { alpha: false });

const state = {
  workers: [],
  ready: 0,
  sceneInfo: null,
  scene: null,
  resolution: 480,
  workerCount: MAX_WORKERS,
  image: null,
  imageData: null,
  queue: [],
  inFlight: 0,
  tilesDone: 0,
  tilesTotal: 0,
  tileTimes: [],
  runStart: 0,
  warmup: false,
  running: false,
};

let requestSeq = 0;
const requestWaiters = new Map();

function post(worker, msg) {
  msg.id = ++requestSeq;
  return new Promise((resolve) => {
    requestWaiters.set(msg.id, resolve);
    worker.postMessage(msg);
  });
}

function onWorkerMessage(msg) {
  if (msg.type === 'ready') {
    state.ready++;
    document.getElementById('worker-count').textContent =
      'spawning workers… ' + state.ready + '/' + state.workers.length;
    if (state.ready === state.workers.length) onAllReady();
    return;
  }
  if (msg.type === 'error') {
    setStatus('engine error: ' + msg.message, true);
    state.running = false;
    return;
  }
  const waiter = requestWaiters.get(msg.id);
  if (waiter) {
    requestWaiters.delete(msg.id);
    waiter(msg);
  }
  if (msg.type === 'bench-tile') onTile(msg);
}

async function onAllReady() {
  const workersSelect = document.getElementById('workers-select');
  for (let i = 1; i <= state.workers.length; i++) {
    const opt = document.createElement('option');
    opt.value = i;
    opt.textContent = i;
    workersSelect.appendChild(opt);
  }
  state.workerCount = DEFAULT_WORKERS;
  workersSelect.value = DEFAULT_WORKERS;
  document.getElementById('worker-count').textContent =
    state.workers.length + ' workers ready';
  await loadScene(BENCH_SCENES[0]);
  setStatus('ready — hit “Run benchmark”');
}

/* ------------------------------------------------------------------ */
/* scene loading                                                       */
/* ------------------------------------------------------------------ */

async function loadScene(entry) {
  state.scene = entry;
  setStatus('loading ' + entry.name + '…');
  const xml = await (await fetch('scenes/' + entry.file + '?v=' + BUILD_VERSION)).text();
  const replies = await Promise.all(state.workers.map((w) =>
    post(w, { type: 'load', xml, height: state.resolution })));
  const first = replies.find((r) => r.type === 'loaded');
  if (!first) {
    setStatus('scene failed to load', true);
    return false;
  }
  state.sceneInfo = first;
  state.image = new Uint32Array(first.width * first.height).fill(0xff000000);
  state.imageData = ctx.createImageData(first.width, first.height);
  canvas.width = first.width;
  canvas.height = first.height;
  document.getElementById('scene-description').textContent = entry.description;
  return true;
}

function benchPayload(tile) {
  const s = state.sceneInfo;
  return {
    type: 'bench',
    tile,
    tilesX: TILES_X,
    tilesY: TILES_Y,
    pass: { aa: s.aa },
    camera: s.camera,
    options: [1, 1, 1, 1, 1, 1, 1, 1],
    quality: [s.lightSamples, s.reflectionDepth, s.refractionDepth, 0],
    aperture: s.aperture,
    dofFocus: 30,
    filter: 0,
  };
}

/* ------------------------------------------------------------------ */
/* benchmark run                                                       */
/* ------------------------------------------------------------------ */

function activeWorkers() {
  return state.workers.slice(0, state.workerCount);
}

function runBenchmark(warmup) {
  state.warmup = warmup;
  state.running = true;
  state.queue = Array.from({ length: TILES_X * TILES_Y }, (_, i) => i);
  state.tilesTotal = state.queue.length;
  state.tilesDone = 0;
  state.tileTimes = [];
  state.inFlight = 0;
  updateProgress(0);
  setStatus(warmup ? 'warm-up run…' : 'measuring — timing the full frame');

  // clock starts at the first dispatch, so worker spawn/compile is excluded
  state.runStart = 0;
  dispatchNext();
}

function onTile(msg) {
  if (!state.running) return;
  const w = state.sceneInfo.width;
  const row = new Uint32Array(msg.pixels);

  // every message carries one freshly rendered scanline of the tile:
  // show it immediately, exactly like the native bench's live preview
  for (let x = 0; x < row.length; x++)
    state.image[msg.rowY * w + msg.x0 + x] = row[x];
  blitRect(msg.x0, msg.rowY, msg.x1, msg.rowY + 1);

  if (!msg.done) return;

  drawTileBorder(msg);
  blitRect(msg.x0, msg.y0, msg.x1, msg.y1);
  state.inFlight--;
  state.tilesDone++;
  state.tileTimes.push(msg.ms);
  updateProgress(state.tilesDone / state.tilesTotal);

  if (state.queue.length) dispatchNext();
  else if (state.inFlight === 0) finishRun();
}

function dispatchNext() {
  const pool = activeWorkers();
  while (state.inFlight < pool.length && state.queue.length) {
    const tile = state.queue.shift();
    const worker = pool[(state.tilesDone + state.inFlight) % pool.length];
    if (state.runStart === 0) state.runStart = performance.now();
    state.inFlight++;
    const msg = benchPayload(tile);
    msg.id = ++requestSeq;
    requestWaiters.set(msg.id, () => {});
    worker.postMessage(msg);
  }
}

function drawTileBorder(msg) {
  // subtle completion mark: one blended teal pixel ring, engine order
  // (0x4FD1C5 displayed -> BGR int 0xC5D14F), mixed 50/50 with the
  // rendered pixel so it reads as a grid hint instead of a highlighter
  const w = state.sceneInfo.width;
  const h = state.sceneInfo.height;
  const img = state.image;
  const blend = (i) => {
    const v = img[i] & 0xffffff;
    img[i] = 0xff000000 |
      ((((v >> 16) & 0xff) + 0x4f) >> 1) << 16 |
      ((((v >> 8) & 0xff) + 0xd1) >> 1) << 8 |
      (((v & 0xff) + 0xc5) >> 1);
  };
  const y0 = msg.y0;
  const y1 = Math.min(msg.y1, h);
  const x0 = msg.x0;
  const x1 = Math.min(msg.x1, w);
  for (let x = x0; x < x1; x++) {
    blend(y0 * w + x);
    if (y1 - 1 > y0) blend((y1 - 1) * w + x);
  }
  for (let y = y0; y < y1; y++) {
    blend(y * w + x0);
    if (x1 - 1 > x0) blend(y * w + x1 - 1);
  }
}

function blitRect(x0, y0, x1, y1) {
  // channel-rotate engine pixels (0x00RRGGBB) into ImageData ([R,G,B,A])
  const w = state.imageData.width;
  const src = state.image;
  const dst = new Uint32Array(state.imageData.data.buffer);
  for (let y = y0; y < y1; y++) {
    for (let x = x0; x < x1; x++) {
      const v = src[y * w + x];
      dst[y * w + x] = 0xff000000 | ((v & 0xff) << 16) | (v & 0xff00) | ((v >>> 16) & 0xff);
    }
  }
  ctx.putImageData(state.imageData, 0, 0, x0, y0, x1 - x0, y1 - y0);
}

function finishRun() {
  state.running = false;
  const durationMs = performance.now() - state.runStart;
  const s = state.sceneInfo;
  const samples = s.width * s.height * (s.aa + 1);
  const mps = samples / 1e6 / (durationMs / 1000);
  const score = Math.round(mps * 10) * 10; // 100 kSamples/s per point

  if (state.warmup) {
    setStatus('warm-up done (' + (durationMs / 1000).toFixed(1) + 's) — measuring…');
    setTimeout(() => runBenchmark(false), 300);
    return;
  }

  const fastest = Math.min(...state.tileTimes);
  const slowest = Math.max(...state.tileTimes);
  document.getElementById('score-value').textContent = score.toLocaleString('en-US');
  document.getElementById('score-details').textContent =
    (durationMs / 1000).toFixed(2) + 's · ' +
    (s.width + '×' + s.height) + ' · AA ×' + (s.aa + 1) + ' · ' +
    mps.toFixed(2) + ' M primary samples/s · ' +
    state.workerCount + (state.workerCount > 1 ? ' workers' : ' worker') +
    ' · tile ' + fastest.toFixed(0) + '–' + slowest.toFixed(0) + ' ms';
  setStatus('done — ' + score.toLocaleString('en-US') + ' pts');

  const history = loadHistory();
  history.unshift({
    date: new Date().toISOString().slice(0, 16).replace('T', ' '),
    scene: state.scene.name,
    res: s.height + 'p',
    workers: state.workerCount,
    seconds: +(durationMs / 1000).toFixed(2),
    score,
  });
  saveHistory(history.slice(0, 20));
  renderHistory();
}

/* ------------------------------------------------------------------ */
/* history                                                             */
/* ------------------------------------------------------------------ */

function loadHistory() {
  try { return JSON.parse(localStorage.getItem('rtbench-history') || '[]'); }
  catch { return []; }
}

function saveHistory(h) {
  try { localStorage.setItem('rtbench-history', JSON.stringify(h)); }
  catch { /* private mode: keep going without history */ }
}

function renderHistory() {
  const box = document.getElementById('history');
  const history = loadHistory();
  if (!history.length) {
    box.innerHTML = '<p class="description">no runs yet</p>';
    return;
  }
  const table = document.createElement('table');
  table.className = 'history-table';
  table.innerHTML =
    '<tr><th>when</th><th>scene</th><th>res</th><th>w</th><th>s</th><th>pts</th></tr>' +
    history.map((r) =>
      '<tr><td>' + r.date + '</td><td>' + r.scene + '</td><td>' + r.res +
      '</td><td>' + r.workers + '</td><td>' + r.seconds + '</td><td>' + r.score + '</td></tr>'
    ).join('');
  box.innerHTML = '';
  box.appendChild(table);
}

/* ------------------------------------------------------------------ */
/* UI                                                                  */
/* ------------------------------------------------------------------ */

function setStatus(text, isError) {
  const el = document.getElementById('status');
  el.textContent = text;
  el.classList.toggle('error', !!isError);
}

function updateProgress(fraction) {
  document.getElementById('progress-fill').style.width = Math.round(fraction * 100) + '%';
}

function buildUi() {
  const sceneSelect = document.getElementById('scene-select');
  for (const s of BENCH_SCENES) {
    const opt = document.createElement('option');
    opt.value = s.file;
    opt.textContent = s.name;
    sceneSelect.appendChild(opt);
  }
  sceneSelect.addEventListener('change', async () => {
    if (state.running) return;
    const entry = BENCH_SCENES.find((s) => s.file === sceneSelect.value);
    if (entry) await loadScene(entry);
  });

  document.getElementById('resolution-select').addEventListener('change', async (e) => {
    if (state.running) return;
    state.resolution = parseInt(e.target.value, 10);
    if (state.scene) await loadScene(state.scene);
  });

  document.getElementById('workers-select').addEventListener('change', (e) => {
    state.workerCount = parseInt(e.target.value, 10);
  });

  document.getElementById('run-button').addEventListener('click', async () => {
    if (state.running || !state.sceneInfo) return;
    document.getElementById('score-value').textContent = '—';
    document.getElementById('score-details').textContent = 'running…';
    runBenchmark(document.getElementById('warmup-check').checked);
  });

  document.getElementById('clear-history').addEventListener('click', () => {
    saveHistory([]);
    renderHistory();
  });

  renderHistory();
}

buildUi();
state.workers = Array.from({ length: MAX_WORKERS }, () => {
  const worker = new Worker('worker.js?v=' + BUILD_VERSION);
  worker.onmessage = (ev) => onWorkerMessage(ev.data);
  return worker;
});
