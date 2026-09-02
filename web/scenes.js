/*
** scene catalog: every entry is a self-contained .xml from rt-scenes/
** (verified to render without external texture assets by web/smoke_test.js)
*/

const SCENES = [
  {
    file: 'demo.xml',
    name: 'Demo',
    description: 'The README example: cone, spheres, ellipsoid and planes with soft shadows.',
  },
  {
    file: 'reflection_tranparency.xml',
    name: 'Glass & mirrors',
    description: 'Reflection, refraction and transparency stacked on curved surfaces.',
  },
  {
    file: 'diapositive.xml',
    name: 'Slide',
    description: 'Light passing through cut objects with smooth shadows.',
  },
  {
    file: 'cubics.xml',
    name: 'Quartics',
    description: 'Torus and other quartic solved shapes (quartic solver showcase).',
  },
  {
    file: 'complex_objects.xml',
    name: 'Complex shapes',
    description: 'Hyperboloids, paraboloids, holo-cubes and pills.',
  },
  {
    file: 'composed_shapes.xml',
    name: 'Composed shapes',
    description: 'Boxes, parallelepipeds and assemblies of primitives.',
  },
  {
    file: 'object_limité.xml',
    name: 'Limited objects',
    description: 'Plane cuts and axis limits carving simple shapes.',
  },
  {
    file: 'perturbation.xml',
    name: 'Perturbation',
    description: 'Procedural marble bump mapping altering the shading.',
  },
  {
    file: 'parallel_light.xml',
    name: 'Parallel light',
    description: 'Directional lighting instead of point lights.',
  },
  {
    file: 'ambiance.xml',
    name: 'Ambiance',
    description: 'Ambient light study.',
  },
  {
    file: 'ambiance++.xml',
    name: 'Ambiance++',
    description: 'Ambient light study, extended.',
  },
  {
    file: 'filters.xml',
    name: 'Filters',
    description: 'Per-pixel color filters (grayscale, sepia, negative).',
  },
  {
    file: 'image_filtering.xml',
    name: 'Image filtering',
    description: 'Kernel post-processing: motion blur, blur, sharpen.',
  },
  {
    file: 'wood_gate.xml',
    name: 'Wood gate',
    description: 'Procedural wood texture on a gate of boxes.',
  },
  {
    file: 'planets.xml',
    name: 'Planets',
    description: 'Planetarium of spheres (file textures absent in the repo, flat shaded).',
  },
  {
    file: 'forest.xml',
    name: 'Forest',
    description: 'Hundreds of trees — the heaviest scene, great for the worker pool.',
  },
];

const DEFAULT_SCENE = 'demo.xml';
