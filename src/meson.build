gtk4_dep = dependency('gtk4')
libportal_dep = dependency('libportal')
gst_dep = dependency('gstreamer-1.0', version : '>=1.19',
    required : true, fallback : ['gstreamer', 'gst_dep'])
gnome = import('gnome')

resources = gnome.compile_resources('ministreamer_resources', 
  'ministreamer.gresource.xml', 
  source_dir: meson.current_source_dir())

src = [
  'MinistreamerGstPipeline.c',
  'ScreenShare.c',
  'MinistreamerWindow.c',
  'MinistreamerApp.c',
  'main.c',
  resources
]

executable('ministreamer',
  src,
  dependencies: [libportal_dep, gst_dep, gtk4_dep],
  c_args: ['-g']
)
