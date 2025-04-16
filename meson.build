project('discord-shell', 'cpp', version : '1.0.0', default_options : ['warning_level=3', 'werror=false', 'cpp_std=c++23'])

deps = [
  dependency('dpp'),
  dependency('zlib'),
]

executable('dsh',
  files(
    'src/bot.cpp',
    'src/crypto/base64.cpp',
    'src/main.cpp',
    'src/spawn/process.cpp',
    'src/zlib.cpp',
  ), 
  dependencies : deps,
  install : true,
)
