import os

import bpy
import mathutils
import bpy_extras.io_utils

BASE_PATH = "/home/malloc/Documents/link_test/"

def bone_id(name: str):
    if(len(name) < 3):
        return None

    bid = name[0:3]
    if(bid[2] != "."):
        return None

    try:
        bid = int(bid[0:2], 16)
    except:
        bid = None
    return bid


def triangulate_mesh(mesh):
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(mesh)
    bm.free()


def write_mesh(object):
    vertices = weights = normals = texuvs = faces = bones = []
    mesh: bpy.types.Mesh = None
    armature: bpy.types.Armature = None

    try:
        mesh = object.to_mesh(bpy.data.scenes[0], False,
            settings='PREVIEW', calc_tessface=False)
    except:
        print("Could not copy mesh.")
        return

    if object.parent != None and object.parent.type == 'ARMATURE':
        armature = object.parent.data
        bones = [(bone_id(group.name), armature.bones[group.name])
            for group in object.vertex_groups]
        check = [bone for bone in bones if bone[0] == None]
        if len(check) > 0:
            for bone in check:
                print("Bone", bone[1].name, "is not named according to format 'XX.NAME'")
            return

    triangulate_mesh(mesh)
    mesh.transform(object.matrix_world)

    if(object.matrix_world.determinant() < 0.0):
        mesh.flip_normals()
    mesh.calc_normals_split()

    vertices = mesh.vertices[:]
    if armature:
        weights = [list(
            (group.group, group.weight)
            for group in vertex.groups)
            for vertex in vertices]
    normals = [vertex.normal for vertex in vertices]


    tris = [(i, tri) for i, tri in enumerate(mesh.polygons)]
    for i, tri in tris:


    bpy.data.meshes.remove(mesh)


def write_armature(object):
    pass

#fw(b"\xDE\xAF\xB0\x0B")

os.makedirs(BASE_PATH, exist_ok=True)
for object in bpy.data.objects:
    if object.type == 'MESH' :
        write_mesh(object)
    elif object.type == 'ARMATURE':
        write_armature(object)
