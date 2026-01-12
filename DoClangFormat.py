import subprocess
import difflib
import os
import sys

file_list = []

if not os.path.exists('.clang-format'):
    print("Error Missing .clang-format configuration file.")
    sys.exit(1)

def file_diff(file1, file2):
    with open(file1, 'r', encoding='utf-8', errors='ignore') as f1, open(file2, 'r', encoding='utf-8', errors='ignore') as f2:
        diff = difflib.unified_diff(
            f1.readlines(),
            f2.readlines(),
            fromfile=file1,
            tofile=file2,
        )
        return ''.join(diff)

source_dirs = [ "./kernel/","./module","./include/"]
for sdirs in source_dirs:
    for root, dirs, files in os.walk(sdirs, topdown=False):
        for name in files:
            file_list.append(os.path.join(root, name))

error_cnt = 0
error_path = []
for itr in range(len(file_list)):
    file_path = file_list[itr]
    if(not os.path.exists(file_path)):
        continue
    if(not (file_path.endswith(".h") or file_path.endswith(".cc") or file_path.endswith(".cu"))):
        continue
    dir_name = file_path.split("/")[1]

    #set clangformat white list

    if(not (dir_name == "kernel" or dir_name == "module" or dir_name == "include")):
        continue
    modified_path = file_path + ".formated"

    #os.system("clang-format -style=file {} > {}".format(file_path, modified_path))
    subprocess.run(["clang-format", "-style=file", file_path], stdout=open(modified_path, 'w'))
    
    if(open(file_path,"rb").read() != open(modified_path,'rb').read()):
        #diff_result = file_diff(file_path, modified_path)
        #print(diff_result)
        print("### diff {} --- {}".format(file_path, modified_path))
        print("******************************start******************************")
        print(os.popen("diff {} {}".format(file_path, modified_path)).read())
        print("*******************************end*******************************")
        error_cnt += 1
        error_path.append(file_path)

    #os.system("rm -r {}".format(modified_path)) 
    os.remove(modified_path)

if(error_cnt):
    print("Get {} files(s) format error(s):".format(error_cnt))
    print(error_path)
    sys.exit(1)
else:
    sys.exit(0)