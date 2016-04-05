{
  "targets": [
    {
	  "target_name": "gomoku",
	  'cflags': ['-Ofast'],
      'cflags_cc': ['-Ofast'],
	  "sources": [ '<!@(ls -1 includes/*.h)', '<!@(ls -1 src/*.cpp)' ]
    }
  ]
}
