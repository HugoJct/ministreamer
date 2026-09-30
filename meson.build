project('gst-template', 'c', version : '0.1', license : 'MIT')

add_languages('cpp', required : true, native : false)

src = [
  'test.c',
]

libportal_dep = dependency('libportal')
gst_dep = dependency('gstreamer-1.0', version : '>=1.19',
    required : true, fallback : ['gstreamer', 'gst_dep'])

executable('testbin',
  src,
  dependencies: [libportal_dep, gst_dep],
)
