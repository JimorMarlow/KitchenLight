import os
import re

def read_version():
    with open('src/version.h', 'r', encoding='utf-8') as f:
        text = f.read()
        major = re.search(r'#define\s+KL_VERSION_MAJOR\s+(\d+)', text).group(1)
        minor = re.search(r'#define\s+KL_VERSION_MINOR\s+(\d+)', text).group(1)
        patch = re.search(r'#define\s+KL_VERSION_PATCH\s+(\d+)', text).group(1)
        version = f"{major}.{minor}.{patch}"
        #print("kitchen light version:", version)
        return version

def sync_file(filename, regex, replacement):
    with open(filename, 'r', encoding='utf-8') as f:
        contents = f.read()
    new_contents = re.sub(regex, replacement, contents)
    with open(filename, 'w', encoding='utf-8') as f:
        f.write(new_contents)

version = read_version()
#sync_file('library.json', r'"version"\s*:\s*"[0-9.]+"', f'"version": "{version}"')
#sync_file('library.properties', r'version=.*', f'version={version}')
sync_file('package.json', r'"version"\s*:\s*"[0-9.]+"', f'"version": "{version}"')
print(f"kitchen light version updated to {version} in metadata files")
exit(0)
