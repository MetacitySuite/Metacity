import os
from metacity.utils.filesystem import read_json


def print_indented(n, s):
        """Print a string `s` indented with `n` tabs at each newline"""
        for x in s.split('\n'):
            print('\t' * n + x)


def inspect_grid(directory: str) -> None:
    """
    Inspects the grid in the given directory.
    """
    
    files = os.listdir(directory)
    if "layout.json" not in files:
        print(f"No layout.json found in directory: {directory}")
        print("Located files:")
        print(files)
        return

    layout_file = os.path.join(directory, "layout.json")
    data = read_json(layout_file)
    print("layout.json:")
    print_indented(1, f"width:     {data['tileWidth']}")
    print_indented(1, f"height:    {data['tileHeight']}")
    print_indented(1, f"num tiles: {len(data['tiles'])}")
    
    print("Tiles:")
    for tile in data['tiles']:
        print_indented(1, f"tile: {tile}")
        file_name = os.path.join(directory, tile['file'])
        data = read_json(file_name)
        print_indented(2, f"{[k for k in data.keys()]}")
        print_indented(2, f"scenes: {len(data['scenes'])}")
        print_indented(2, f"nodes: {len(data['nodes'])}")
        print_indented(2, f"meshes: {len(data['meshes'])}")
        print_indented(2, f"accessors: {len(data['accessors'])}")
        print_indented(2, f"bufferViews: {len(data['bufferViews'])}")
        print_indented(2, f"buffers: {len(data['buffers'])}")
        for node in data['nodes']:
            mesh_id = node['mesh']
            for primitive in data['meshes'][mesh_id]['primitives']:
                accessor_id = primitive['attributes']['POSITION']
                accessor = data['accessors'][accessor_id]
                span = [ b - a for a, b in zip(accessor['min'], accessor['max']) ]
                if any(x < 0 for x in span):
                    print_indented(3, f"{mesh_id} has negative span: {span}")
                if accessor['count'] == 0:
                    print_indented(3, f"{mesh_id} has zero count")
            