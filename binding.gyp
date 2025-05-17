{
  "targets": [
    {
      "target_name": "vddk",
      "sources": [ "src/vddk-wrapper.cpp" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "lib/include",
        "lib/include/vmware-vix-disklib"
      ],
      "libraries": [
        "-L/home/florent/Documents/node-vddk/lib/lib64",
        "/home/florent/Documents/node-vddk/lib/lib64/libvixDiskLib.so",
      ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "conditions": [
        ['OS=="linux"', {
          "libraries": [
            "-ldl",
            "-lpthread",
            "-lssl",
            "-lcrypto",
            "-lxml2",
            "-lcurl"
          ],
          "ldflags": [
            "-Wl,-rpath,<(module_root_dir)/lib/lib64"
          ]
        }]
      ]
    }
  ]
}