import sys

hid = sys.argv[2]
path = sys.argv[1]

def merge_profile(path: str, hid: str, profile_id: int) -> (int, str):
    buffer = b''
    for i in range(0, 3):
        with open('%s/report_%s_%d.bin' % (path, hid, profile_id + i), 'rb') as f:
            buffer += f.read()[2:-4]
    name = buffer[4:84].decode('utf-16').split('\0')[0]
    print(name)
    with open('%s.bin' % name.replace('/', '-').replace('<', '-').replace('>', '-').replace('\\', '-'), 'w+b') as f:
        f.write(buffer)

merge_profile(path, hid, 0x73)
merge_profile(path, hid, 0x76)
merge_profile(path, hid, 0x79)
