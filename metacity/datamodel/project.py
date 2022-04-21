from metacity.filesystem import layer as fs
from metacity.datamodel.layer import Layer

class Project:
    def __init__(self, directory: str):
        """
        Initializes the project. The project is created as a new directory.

        Args:
            directory (str): The directory of the project.
        
        Returns:
            Project: The project.
        """
        self.dir = directory
        fs.create_project(self.dir)

    def create_layer(self, layer_name: str):
        """
        Creates a new layer with the given name.
        params:
            layer_name (str): The name of the layer.
        
        returns:
            Layer: The created layer.
        """
        layer_dir = fs.non_coliding_layer_dir(self.dir, layer_name)
        layer = Layer(layer_dir)
        return layer

    def get_layer(self, layer_name: str, load_set=True, load_meta=True, load_geometry=True):
        """
        Gets a layer by its name. If the layer does not exist, it is created. 

        Args:
            layer_name (str): The name of the layer.
            load_set (bool, optional): If the layer should be loaded. Defaults to True.

        Returns:
            Layer: The layer.
        """
        layer_dir = fs.layer_dir(self.dir, layer_name)
        layer = Layer(layer_dir, load_set=load_set, load_meta=load_meta, load_geometry=load_geometry)
        return layer

    def delete_layer(self, layer_name: str):
        """
        Deletes a layer by its name.

        Args:
            layer_name (str): The name of the layer.
        """
        layer_dir = fs.layer_dir(self.dir, layer_name)
        fs.base.delete_dir(layer_dir)

    def delete_overlay(self, overlay_name: str):
        """
        Deletes a overlay by its name.

        Args:
            overlay_name (str): The name of the overlay.
        """
        self.delete_layer(overlay_name)
        

    def rename_layer(self, old_name: str, new_name: str):
        """
        Renames a layer.

        Args:
            old_name (str): The old name of the layer.
            new_name (str): The new name of the layer.

        Returns:
            bool: True if the layer was renamed, False otherwise.
        """
        old_layer_dir = fs.layer_dir(self.dir, old_name)
        new_layer_dir = fs.layer_dir(self.dir, new_name)
        if not fs.base.valid_name(new_name):
            return False
        return fs.base.rename(old_layer_dir, new_layer_dir)            

    def delete(self):
        """
        Deletes the project.
        """
        fs.base.delete_dir(self.dir)


    @property
    def layer_names(self):
        """
        Gets the names of all available project layers.

        Returns:
            list: The names of all layers.
        """
        return fs.layer_names(self.dir)

    @property
    def layers(self):
        """
        Generator, yields all layers. 

        Returns:
            list: All layers.
        """
        names = self.layer_names
        for name in names:
            self.get_layer(name)


    def clayers(self, load_set=True, load_meta=True, load_geometry=True):
        """
        Generator,  yields all layers, parametrizable:

        Args:
            load_set (bool, optional): If the objects should be loaded. Defaults to True.
            load_meta (bool, optional): If the meta data should be loaded. Defaults to True.
            load_geometry (bool, optional): If the geometry should be loaded. Defaults to True.
        """
        names = self.layer_names
        for name in names:
            self.get_layer(name, load_set=load_set, load_meta=load_meta, load_geometry=load_geometry)


