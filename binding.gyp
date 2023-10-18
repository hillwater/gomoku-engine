{
  "targets": [
    {
	  "target_name": "gomoku",
	  'cflags': ['-Ofast'],
    'cflags_cc': ['-Ofast'],
    "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
    ],
	  "sources": [ '<!@(ls -1 includes/*.h)', '<!@(ls -1 src/*.cpp)' ],
    'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
    }
  ]
}
