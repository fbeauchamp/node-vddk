{
  "targets": [
    {
      "target_name": "vddk",
      "sources": [ "src/vddk-wrapper.cpp" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "lib/include",
        "disklib/include"
      ],
      'link_settings': {
      	"library_dirs": [
        	"/vagrant/disklib/lib64",
      	],
	    "libraries": [ '-lvixDiskLib', "-ldl" ]
      },
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      'cflags': ['-fPIC', '-L ./disklib/lib64'],
      "conditions": [
        ['OS=="linux"', {
          "ldflags": [
            "-Wl,-rpath=/vagrant/disklib/lib64"
          ]
        }]
      ]
    }
  ]
}
