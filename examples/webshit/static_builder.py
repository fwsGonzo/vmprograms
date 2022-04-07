import os
import mimetypes
import zlib

location = "res"
mime = mimetypes.MimeTypes()

file = open("/tmp/static_builder.c", "w")
file.write('#include "../varnish.h"\n')
file.write('#include "crc32.h"\n')
file.write('#include <stdio.h>\n')
file.write('\n')

binaries = []
cases = []
bins = 0

for path, subdirs, files in os.walk(location):
    for name in files:
        fullpath = os.path.join(path, name)

        ## embed file as resource0, resource1 etc.
        bname = "resource" + str(bins)
        bins = bins + 1
        binaries.append('EMBED_BINARY(' + bname + ', "' + fullpath + '")\n')

        ## create CRC32 of /resource/path
        rel = os.path.relpath(fullpath, "res")
        resname = bytes("/" + rel, 'utf-8')
        crc = hex(zlib.crc32(resname) & 0xffffffff)
        ## mimetype deduction
        ext = os.path.splitext(fullpath)
        if ext == '.css':
            mimetype = "text/css"
        elif ext == '.woff2':
            mimetype = "font/woff2"
        else:
            mimetype = mime.guess_type(name)[0]
            if mimetype is None:
                mimetype = "text/plain"
        print(rel, crc, mimetype)
        ## create switch case
        cases.append('  case ' + crc + ': {\n')
        cases.append('    const char ctype[] = "' + mimetype + '";\n')
        cases.append('    backend_response(200, ctype, sizeof(ctype)-1, ' + bname + ', ' + bname + '_size);\n')
        cases.append('  } break;\n')

for bin in binaries:
    file.write(bin)
file.write('\n')

file.write('static void my_backend(const char *arg, int req, int resp)\n')
file.write('{\n')
file.write('  const uint32_t crc = crc32(arg);\n')
file.write('  switch (crc) {\n')
for case in cases:
    file.write(case)
file.write('  default:\n')
file.write('    printf("Unknown resource requested: %s (crc: 0x%X)\\n",\n')
file.write('        arg, crc);\n')
file.write('    const char ctype[] = "text/plain";\n')
file.write('    const char cont[] = "Resource not found";\n')
file.write('    backend_response(404, ctype, sizeof(ctype)-1, cont, sizeof(cont)-1);\n')
file.write('  } /* switch(crc) */\n')
file.write('} /* my_backend */\n')

bincount = str(len(binaries))
file.write('\n')
file.write('int main(int argc, char** argv) {\n')
file.write('  printf("Static resource builder with ' + bincount + ' objects initialized!\\n");\n')
file.write('  set_backend_get(my_backend);\n')
file.write('  wait_for_requests();\n')
file.write('}\n')

file.close()
