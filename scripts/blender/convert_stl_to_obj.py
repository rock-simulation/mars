import os
import bpy
import mathutils

def convertStlToObj(path) :
  # change the current working directory
  os.chdir(path)
  # get a list of all files within this directory
  files = os.listdir('.')
  # check each entry if it is a file or a directory
  for file in files:
    # handle a file
    if file.endswith('.stl') or file.endswith('.STL'):
      # clear the current scene
      bpy.ops.scene.new()
      # import the stl mesh
      bpy.ops.import_mesh.stl(filepath=file)
      #export it as an obj scene
      bpy.ops.export_scene.obj(filepath=file.replace('.stl','.obj').replace('.STL','.obj'))
     # clean up the newly cerated scene
      bpy.ops.scene.delete()
    # handle a directory
    if os.path.isdir(file) :
      # recursively convert the files 
      convertStlToObj(path + os.sep + file)
      # reset the current directory
      os.chdir(path)

convertStlToObj("...")