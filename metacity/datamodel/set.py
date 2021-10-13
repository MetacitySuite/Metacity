from typing import Callable, Dict, List, Tuple
from metacity.datamodel.object import Object, desermodel
from metacity.filesystem import layer as fs
from metacity.filesystem import grid as fs
from metacity.geometry.primitive import MultiPoint, MultiLine, MultiPolygon, Primitive
from metacity.utils.persistable import Persistable




class DataSet(Persistable):
    def __init__(self, set_dir: str, offset: int, capacity: int):
        self.set_dir = set_dir
        fs.base.create_dir_if_not_exists(set_dir)
        self.path = fs.data_set(self.set_dir, offset)

        super().__init__(self.path)

        self.offset: int = offset
        self.capacity: int = capacity
        self.data = []

        try:
            self.load()
        except IOError:
            self.export()

    @property
    def full(self):
        return self.capacity <= len(self.data)

    def contains(self, index: int):
        return index >= self.offset and index < self.offset + len(self.data)

    def can_contain(self, index: int):
        return (index >= self.offset) and (index < (self.offset + self.capacity))

    def add(self, item):
        self.data.append(item)

    def __getitem__(self, index: int):
        if not self.contains(index):
            raise IndexError(f"No object at index {index}")
        return self.data[index - self.offset]

    def serialize(self):
        return {
            'capacity': self.capacity,
            'offset': self.offset,

        }

    def deserialize(self, data):
        self.capacity = data['capacity']
        self.offset = data['offset']


types: Dict[str, Callable[[],Primitive]] = {
    MultiPoint().type: MultiPoint,
    MultiLine().type: MultiLine,
    MultiPolygon().type: MultiPolygon
}


def desermodel(model):
    type = model["type"]
    if model["type"] in types:
        m = types[type]()
    else:
        raise RuntimeError(f"Unknown model type: {type}")
    m.deserialize(model)
    return m


class ModelSet(DataSet):
    def __init__(self, layer_dir: str, offset: int, capacity: int):        
        super().__init__(fs.layer_models(layer_dir), offset, capacity)

    def serialize(self): 
        model: Primitive
        data = super().serialize()
        objects = []
        for object in self.data:
            models = []
            for model in object:
                models.append(model.serialize())
            objects.append(models)
        data['models'] = objects
        return data

    def deserialize(self, data):
        super().deserialize(data)
        self.data = []
        for object in data['models']:
            models = []
            for model in object:
                models.append(desermodel(model))
            self.data.append(models)


class MetaSet(DataSet):
    def __init__(self, layer_dir: str, offset: int, capacity: int):        
        super().__init__(fs.layer_metadata(layer_dir), offset, capacity)

    def serialize(self): 
        data = super().serialize()
        data['meta'] = self.data
        return data

    def deserialize(self, data):
        super().deserialize(data)
        self.data = data['meta']


class ObjectSet:
    def __init__(self, layer_dir: str, offset: int, capacity: int):   
        self.models = ModelSet(layer_dir, offset, capacity)
        self.meta = MetaSet(layer_dir, offset, capacity)

    def can_contain(self, index: int):
        return self.models.can_contain(index) 

    def add(self, object: Object):
        self.models.add(object.models)
        self.meta.add(object.meta)

    def __getitem__(self, index: int):
        if not self.models.contains(index):
            raise IndexError(f"No object at index {index}")
        obj = Object()
        obj.models = self.models[index]
        obj.meta = self.meta[index] 
        return obj

    def export(self):
        self.models.export()
        self.meta.export()


class TileSet(DataSet):
    def __init__(self, grid_dir: str, tile_name: str, offset: int, capacity: int):        
        super().__init__(fs.grid_cache_tile_dir(grid_dir, tile_name), offset, capacity)

    def serialize(self): 
        data = super().serialize()
        models = []
        for model in self.data:
            models.append(model)
        data['models'] = models
        return data

    def deserialize(self, data):
        super().deserialize(data)
        self.data = []
        for oid, model in data['models']:
            self.data.append([oid, desermodel(model)])

    def add(self, oid, model):
        super().add([oid, model])




