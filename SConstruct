import sys
try:
  import sconsutils
except ImportError:
  raise ImportError("Failed to import sconsutils (is buildtools on PYTHONPATH?)"), None, sys.exc_traceback

env = sconsutils.getEnv()
env.InVariantDir(env['oDir'], ".", lambda env: env.LibAndApp('iterators', 0, -1, (
  ('core', 0, 0),
)))
