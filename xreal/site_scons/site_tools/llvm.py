import os
import os.path
import string

import SCons.Action
import SCons.Builder
import SCons.Defaults
import SCons.Tool
import SCons.Util
import SCons.Scanner.Prog
ProgramScanner = SCons.Scanner.Prog.ProgramScanner()
SourceFileScanner = SCons.Scanner.Base({}, name = 'SourceFileScanner')
CScanner = SCons.Scanner.C.CScanner()
CSuffixes = [".c", ".C", ".cxx", ".cpp", ".c++", ".cc",
             ".h", ".H", ".hxx", ".hpp", ".hh",
             ".F", ".fpp", ".FPP",
             ".m", ".mm",
             ".S", ".spp", ".SPP"]

for suffix in CSuffixes:
    SourceFileScanner.add_scanner(suffix, CScanner)


LLVMCompileAction = SCons.Action.Action("llvm-gcc $CCFLAGS -emit-llvm $SOURCE -o $TARGET")
LLVMLinkAction = SCons.Action.Action("llvm-link $SOURCES -o $TARGET")

"""
try:
    c_file = env['BUILDERS']['CFile']
except KeyError:
    c_file = SCons.Builder.Builder(action = [LLVMCompileAction],
                                   emitter = {},
                                   suffix = {None:'$CFILESUFFIX'})
    env['BUILDERS']['CFile'] = c_file

    env.SetDefault(CFILESUFFIX = '.c')


try:
    LLVMByteCodeObject = env['BUILDERS']['LLVMByteCodeObject'] 
except KeyError:
    import SCons.Defaults
    LLVMByteCodeObject = SCons.Builder.Builder(action = [LLVMCompileAction],
                                       emitter = {},
                                       prefix = '$SHOBJPREFIX',
                                       suffix = '$SHOBJSUFFIX',
                                       src_builder = ['CFile', 'CXXFile'],
                                       source_scanner = SourceFileScanner,
                                       single_source = 1)
    env['BUILDERS']['LLVMByteCodeObject'] = LLVMByteCodeObject
"""


def generate(env):

    try:
        LLVMModuleBuilder = env['BUILDERS']['LLVMModule']
    except KeyError:
        import SCons.Defaults
        action_list = [ LLVMLinkAction ]
        LLVMModuleBuilder = SCons.Builder.Builder(action = action_list,
                                           emitter = "$SHLIBEMITTER",
                                           prefix = '$SHLIBPREFIX',
                                           suffix = '$SHLIBSUFFIX',
                                           target_scanner = ProgramScanner,
                                           src_suffix = '$SHOBJSUFFIX',
                                           src_builder = 'SharedObject')
        env['BUILDERS']['LLVMModule'] = LLVMModuleBuilder

def exists(env):
    return find(env)

