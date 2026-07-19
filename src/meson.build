src = [
  'main.c',
  'ScreenShare.c',
]

gtk4_dep = dependency('gtk4')
libportal_dep = dependency('libportal')
gst_dep = dependency('gstreamer-1.0', version : '>=1.19',
    required : true, fallback : ['gstreamer', 'gst_dep'])

executable('ministreamer',
  src,
  dependencies: [libportal_dep, gst_dep, gtk4_dep],
  c_args: ['-g']
)
