import os
from shutil import copyfile

Import("env")
pioEnv = env['PIOENV']

def copyFirmware(source, target, env):
  print ( "------------------------------------------------------------------------------" )

  firmwarePath = os.path.abspath(os.path.join( os.path.join( target[0].path, os.pardir), ( "firmware-%s.bin" % ( pioEnv ))))
 
  print ( "PIOENV = " + pioEnv )
  print ( "copy firmware to file:" )
  print ( firmwarePath )
  copyfile( target[0].path, firmwarePath )
  print ( "size=%d bytes" % os.path.getsize( firmwarePath ))

env.AddPostAction( "$BUILD_DIR/firmware.bin", copyFirmware )
