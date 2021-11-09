from metacity.datamodel.grid import Grid
from metacity.datamodel.object import Object
from metacity.datamodel.set import ObjectSet
from metacity.filesystem import layer as fs
from metacity.utils.persistable import Persistable
from tqdm import tqdm


class Layer(Persistable):
    def __init__(self, layer_dir: str, group_by = 100000, load_set=True):
        super().__init__(fs.layer_config(layer_dir))

        self.dir = layer_dir
        self.size = 0
        self.group_by = group_by

        fs.create_layer(self.dir)

        try:
            self.load()
        except FileNotFoundError:
            self.export()

        if load_set:
            self.set = ObjectSet(self.dir, 0, self.group_by)
        else:
            self.set = None
    
    @property
    def type(self):
        return "layer"

    @property
    def name(self):
        return fs.layer_name(self.dir)

    @property
    def grid(self):
        return Grid(self.dir)

    def add(self, object: Object):
        if not self.set.can_contain(self.size):
            self.set.export()
            self.activate_set(self.size)
        self.set.add(object)
        self.size += 1

    def persist(self):
        self.export()
        self.set.export()

    def __getitem__(self, index: int):
        if not self.set.can_contain(index):
            self.set.export()
            self.activate_set(index)
        obj = self.set[index]
        return obj

    def activate_set(self, index):
        offset = (index // self.group_by) * self.group_by
        self.set = ObjectSet(self.dir, offset, self.group_by)

    def regroup(self, group_by):
        tmp = fs.layer_regrouped(self.dir)
        fs.remove(tmp)

        regrouped = Layer(tmp, group_by)
        for o in tqdm(self.objects):
            regrouped.add(o)

        regrouped.persist()
        fs.move_from_regrouped(self.dir)
        self.group_by = group_by
        self.export()

    @property
    def meta(self):
        return LayerMetaIterator(self)

    @property
    def objects(self):
        for i in range(self.size):
            yield self[i]

    def add_source_file(self, source_file_path: str):
        return fs.copy_to_layer(self.dir, source_file_path)

    def build_grid(self):
        grid = Grid(self.dir)
        grid.clear()
        for oid, object in tqdm(enumerate(self.objects)):
            for model in object.models:
                grid.add(oid, model) 
        return grid

    def serialize(self):
        return {
            'type': 'layer',
            'size': self.size,
            'group_by': self.group_by
        }

    def deserialize(self, data):
        self.size = data['size']
        self.group_by = data['group_by']

    def build_layout(self):
        grid = self.grid
        if grid.init:
            return {
                'name': self.name,
                'layout': grid.build_layout(),
                'size': self.size,
                'init': True,
                'type': 'layer'
            }
        else:
            return {
                'name': self.name,
                'init': False,
                'type': 'layer'
            }


class LayerMetaIterator:
    def __init__(self, layer: Layer):
        self.dir = layer.dir
        self.size = layer.size
        self.group_by = layer.group_by
        self.index = 0
        self.set = ObjectSet(self.dir, 0, layer.group_by, load_model=False)

    def activate_set(self, index):
        offset = (index // self.group_by) * self.group_by
        self.set = ObjectSet(self.dir, offset, self.group_by, load_model=False)

    def __iter__(self):
        return self

    def __next__(self):
        if self.index >= self.size:
            raise StopIteration
        if not self.set.can_contain(self.index):
            self.activate_set(self.index)
        obj = self.set[self.index]
        self.index += 1
        return obj.meta



class LayerOverlay(Persistable):
    def __init__(self, overlay_dir: str):
        super().__init__(fs.layer_config(overlay_dir))
        self.source_layer = None
        self.target_layer = None
        self.dir = overlay_dir
        fs.create_overlay(self.dir)

        try:
            self.load()
        except FileNotFoundError:
            self.export()

    @property
    def type(self):
        return "overlay"

    @property
    def grid(self):
        return Grid(self.dir)

    @property
    def name(self):
        return fs.layer_name(self.dir)

    def setup(self, source: Layer, target: Layer):
        if source.type != "layer" or target.type != "layer":
            raise Exception(f"Cannot map type {source.type} to {target.type}, only layer to layer is supported")

        tg = target.grid
        sg = source.grid

        grid = self.grid

        for source_tile, target_tile in sg.overlay(tg):
            pol = target_tile.polygon
            if pol is None:
                continue
            for source_model in source_tile.objects:
                source_copy = source_model.copy()
                source_copy.map(pol)
                grid.tile_from_single_model(source_copy, source_tile.name)

        grid.persist() #persist with empy cache
        self.source_layer = source.name
        self.target_layer = target.name

    def persist(self):
        self.export()

    def serialize(self):
        return {
            'type': 'overlay',
            'source': self.source_layer,
            'target': self.target_layer
        }

    def deserialize(self, data):
        self.source_layer = data['source']
        self.target_layer = data['target']

    def build_layout(self):
        grid = self.grid
        if grid.init:
            return {
                'name': self.name,
                'source': self.source_layer,
                'target': self.target_layer,
                'layout': grid.build_layout(),
                'init': True,
                'type': 'overlay'
            }
        else:
            return {
                'name': self.name,
                'source': self.source_layer,
                'target': self.target_layer,
                'init': False,
                'type': 'overlay'
            }





