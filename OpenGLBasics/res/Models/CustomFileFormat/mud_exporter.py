import bpy

#############################################################
#                   STRUCTURES                              #
#############################################################

class Tag:
    def __init__(self, name, attributes, values):
        self.name = name
        self.attributes = attributes
        self.values = values
        self.children = []

class BoneTag(Tag):
    def __init__(self):
        super().__init__('Bone', ['name', 'translation', 'rotation'], ['tag', '0.0 0.0 0.0', '0.0 0.0 0.0 0.0'])

class PropTag(Tag):
    def __init__(self, name, value):
        super().__init__(name, ['value'], [value])
        
class NamedTag(Tag):
    def __init__(self, name, nameValue):
        super().__init__(name, ['name'], [nameValue])


class SimpleTag(Tag):
    def __init__(self, name):
        super().__init__(name, [], [])




#############################################################
#                   FUNCTIONS                               #
#############################################################

def writeTag(file, tag, level):
    for i in range(level):
        file.write("\t")
    file.write('<' + tag.name)
    
    for attribute, value in zip(tag.attributes, tag.values):
        file.write(' ' + attribute + '="' + value +'"')

    file.write(">\n")
        
    
    for child in tag.children:
        writeTag(file, child, level + 1)
            
    for i in range(level):
        file.write("\t")
    file.write("</" + tag.name + ">\n")
    return
    
def buildTree():
    selected_object = bpy.context.object
    mesh = bpy.data.meshes[selected_object.name]
    # Triangulate mesh faces
    mesh.calc_loop_triangles()
    # To access faces use "mesh.loop_triangles"
    rootNode = NamedTag('model', mesh.name)
    
    meshNode = NamedTag('mesh', mesh.name)
    meshNode.attributes.append('vertexcount')
    meshNode.values.append(str(len(mesh.vertices)))
    
    rootNode.children.append(meshNode)
    
    
    for vertex in mesh.vertices:
        vertexNode = Tag('vertex', ['id'], [str(vertex.index)])
        
        #position
        vertPos = vertex.co
        value = str(vertPos.x) + ' ' + str(vertPos.y) + ' ' + str(vertPos.z)
        position  = PropTag('position', value)
        vertexNode.children.append(position)
        
        #normal
        vertNormal = vertex.normal
        value = str(vertNormal.x) + ' ' + str(vertNormal.y) + ' ' + str(vertNormal.z)
        normal  = PropTag('normal', value)
        vertexNode.children.append(normal)
        
        #bones indices and weight
        groups = iter(vertex.groups)
        firstG = next(groups)
        
        value = str(firstG.group)
        wValues = str(firstG.weight)
        
        for group in groups:
            value += ' '
            value += str(group.group)
            wValues += ' '
            wValues += str(group.weight)
        
        indices = PropTag('indices', value)
        weights = PropTag('weights', wValues)
        
        vertexNode.children.append(indices)
        vertexNode.children.append(weights)
        
        #add vertex to mesh node
        meshNode.children.append(vertexNode)
    
    # mesh indices
    indicesNode = Tag('indices', ['count', 'values'],\
                  [str(len(mesh.loop_triangles) * 3), ""])
    values = ""
    
    for triangle in mesh.loop_triangles:
        for index in triangle.vertices:
            values += ' ' + str(index)
    values = values[1:]
    
    indicesNode.values[1] = values
    meshNode.children.append(indicesNode)
    
    return rootNode

def write_some_data(context, filepath):
    print("writing file...")
    f = open(filepath, 'w', encoding='utf-8')
    
    tree = buildTree()
    writeTag(f, tree, 0)
    
    f.close()
    print("model exported successfuly...")
    return {'FINISHED'}


# ExportHelper is a helper class, defines filename and
# invoke() function which calls the file selector.
from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy.types import Operator


class ExportSomeData(Operator, ExportHelper):
    """Export the selected model and armature to MUD Renderer format"""
    bl_idname = "export_mud_model.some_data"  # important since its how bpy.ops.import_test.some_data is constructed
    bl_label = "Export MUD model and skeleton"

    # ExportHelper mixin class uses this
    filename_ext = ".mudm"

    filter_glob: StringProperty(
        default="*.mudm",
        options={'HIDDEN'},
        maxlen=255,  # Max internal buffer length, longer would be clamped.
    )

    # List of operator properties, the attributes will be assigned
    # to the class instance from the operator settings before calling.
#    use_setting: BoolProperty(
#        name="Example Boolean",
#        description="Example Tooltip",
#        default=True,
#    )

    type: EnumProperty(
        name="Up Axis",
        description="Choose wich axis is the up direction",
        items=(
            ('OPT_A', "Z AXIS", "Z Axis as up vector"),
            ('OPT_B', "Y AXIS", "Y Axis as up vector"),
            ('OPT_C', "X AXIS", "X Axis as up vector"),
        ),
        default='OPT_A',
    )

    def execute(self, context):
        return write_some_data(context, self.filepath)


# Only needed if you want to add into a dynamic menu
def menu_func_export(self, context):
    self.layout.operator(ExportSomeData.bl_idname, text="Text Export Operator")


def register():
    bpy.utils.register_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()

    # test call
    bpy.ops.export_mud_model.some_data('INVOKE_DEFAULT')
