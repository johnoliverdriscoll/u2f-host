{
  "targets": [{
    "target_name": "U2fHost",
    "sources": [
      "U2fHost.cc"
    ],
    "include_dirs" : [
      "<!(node -e \"require('nan')\")",
      "include"
    ],
    "libraries": [
      "-l:libu2f-host.so.0"
    ]
  }]
}
