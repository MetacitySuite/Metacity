import os
import shutil

class DirectoryTreePaths:
    def __init__(self, output_dir):
        self.output = output_dir
        self.objects = os.path.join(self.output, "objects")
        self.geometry = os.path.join(self.output, "geometry")
        self.point_geometry = os.path.join(self.geometry, "points")
        self.line_geometry = os.path.join(self.geometry, "line")
        self.facet_geometry = os.path.join(self.geometry, "facets")
        self.stl = os.path.join(self.output, "stl")


    def recreate_tree(self):
        if os.path.exists(self.output):
            shutil.rmtree(self.output)
        os.mkdir(self.output)
        os.mkdir(self.objects)
        os.mkdir(self.geometry)
        os.mkdir(self.point_geometry)
        os.mkdir(self.line_geometry)
        os.mkdir(self.facet_geometry)
        os.mkdir(self.stl)


    def recreate_stl(self):
        if os.path.exists(self.stl):
            shutil.rmtree(self.stl)    
        os.mkdir(self.stl)


    @property
    def primitive_dirs(self):
        """List of primitive directories relative to self.geometry directory

        Returns:
            List[str]: List of directory names
        """
        return [f for f in os.listdir(self.geometry) if os.path.isdir(os.path.join(self.geometry, f))]


    @property
    def lod_dirs(self):
        """List of LODs inside primitives relative to self.geometry directory.

        Returns:
            List[str]: List of directory names
        """
        primitives = self.primitive_dirs
        dirs = []
        for primitive in primitives:
            primitive_dir = os.path.join(self.geometry, primitive) 
            for lod_dir in os.listdir(primitive_dir):
                absolute_lod_dir = os.path.join(primitive_dir, lod_dir)
                if os.path.isdir(absolute_lod_dir):
                    dirs.append(os.path.join(primitive, lod_dir))
        return dirs
    

    def use_lod_directory(self, base, lod):
        lod_dir = os.path.join(base, str(lod))
        if not os.path.exists(lod_dir):
            os.mkdir(lod_dir)
        return lod_dir


    def use_points_lod_directory(self, lod):
        return self.use_lod_directory(self.point_geometry, lod)


    def use_line_lod_directory(self, lod):
        return self.use_lod_directory(self.line_geometry, lod)


    def use_facet_lod_directory(self, lod):
        return self.use_lod_directory(self.facet_geometry, lod)