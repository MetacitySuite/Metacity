from numpy import number
from metacity.utils.filesystem import read_json
from metacity.geometry import Attribute, Model
from metacity.geometry import Progress
from typing import List
from pyproj import Proj, Transformer


__all__ = ["parse", "parse_data"]


class Geometry:
    def __init__(self, data):
        self.data = data
        self.transform = None

    @property
    def dim(self):
        d = self.coordinates
        while isinstance(d[0], list) or isinstance(d[0], tuple):
            d = d[0]
        d = len(d)
        if d < 2 and d > 3:
            raise Exception(
                f"Encountered primitive with unsupported dimension {d}")
        return d

    def set_projection(self, from_crs: str, to_crs: str):
        if from_crs is None or to_crs is None:
            return

        proj_from = Proj(from_crs, preserve_units=False)
        proj_to = Proj(to_crs, preserve_units=False)
        self.transform = Transformer.from_proj(proj_from, proj_to)


    def project(self, coordinates: List[List[float]]):
        if self.transform is None:
            return coordinates
        return [ p for p in self.transform.itransform(coordinates) ]

    @property
    def coordinates(self):
        return self.data['coordinates']

    @property
    def geometry_type(self):
        return self.data['type'].lower()

    @property
    def geometries(self):
        return [Geometry(g) for g in self.data['geometries']]


class Feature:
    def __init__(self, data):
        self.geometry = Geometry(data['geometry'])
        if 'properties' in data:
            self.properties = data['properties']
        else:
            self.properties = { 'data': None } 


def flatten(data):
    return [item for sublist in data for item in sublist]


def flatten_polygon(polygon):
    return [[item for sublist in ring for item in sublist] for ring in polygon]


def to_model(attr: Attribute):
    model = Model()
    model.add_attribute("POSITION", attr)
    return model


###############################################################################


def parse_point(g: Geometry, attr: Attribute, dim: int, vertices: List[List[float]]):
    vertices = g.project(vertices)
    vertices = flatten(vertices)
    if dim == 2:
        attr.push_point2D(vertices)
    elif dim == 3:
        attr.push_point3D(vertices)


def parse_line(g: Geometry, attr: Attribute, dim: int, line: List[List[float]]):
    l = g.project(line)
    l = flatten(line)
    if dim == 2:
        attr.push_line2D(l)
    elif dim == 3:
        attr.push_line3D(l)


def parse_polygon(g: Geometry, attr: Attribute, dim: int, polygon: List[List[List[float]]]):
    polygon = [ g.project(p) for p in polygon ]
    p = flatten_polygon(polygon)
    if dim == 2:
        attr.push_polygon2D(p)
    elif dim == 3:
        attr.push_polygon3D(p)


###############################################################################


def model_from_point(geometry: Geometry):
    attr = Attribute()
    vertices = [geometry.coordinates]
    parse_point(geometry, attr, geometry.dim, vertices)
    return [to_model(attr)]


def model_from_multipoint(geometry: Geometry):
    attr = Attribute()
    vertices = geometry.coordinates
    parse_point(geometry, attr, geometry.dim, geometry.coordinates)
    return [to_model(attr)]


def model_from_linestring(geometry: Geometry):
    attr = Attribute()
    vertices = geometry.coordinates
    parse_line(geometry, attr, geometry.dim, vertices)
    return [to_model(attr)]


def model_from_multilinestring(geometry: Geometry):
    attr = Attribute()
    dim = geometry.dim
    for line in geometry.coordinates:
        parse_line(geometry, attr, dim, line)
    return [to_model(attr)]


def model_from_polygon(geometry: Geometry):
    attr = Attribute()
    vertices = geometry.coordinates
    parse_polygon(geometry, attr, geometry.dim, vertices)
    return [to_model(attr)]


def model_from_multipolygon(geometry: Geometry):
    attr = Attribute()
    dim = geometry.dim
    for polygon in geometry.coordinates:
        parse_polygon(geometry, attr, dim, polygon)
    return [to_model(attr)]


def model_from_geometrycollection(geometry: Geometry):
    models = []
    for subgeometry in geometry.geometries:
        model_list = typedict[subgeometry.geometry_type](subgeometry)
        models.extend(model_list)
    return models


typedict = {
    "point": model_from_point,
    "multipoint": model_from_multipoint,
    "linestring": model_from_linestring,
    "multilinestring": model_from_multilinestring,
    "polygon": model_from_polygon,
    "multipolygon": model_from_multipolygon,
    "geometrycollection": model_from_geometrycollection
}


def parse_feature(feature: Feature, from_crs: str, to_crs: str, progress: Progress = None):
    #try:
    feature.geometry.set_projection(from_crs, to_crs)
    model_list = typedict[feature.geometry.geometry_type](feature.geometry)
    for model in model_list:
        if progress is not None:
            progress.update()
        model.set_metadata(feature.properties)
    return model_list
    #except:
    #    message = f"""
    #    Skipping unsupported or empty features:
    #        type: {feature.geometry.geometry_type}
    #    """
    #    print(message)
    return []


def parse_data(data, from_crs: str, to_crs: str, progress: Progress = None):
    models = []
    for f in data['features']:
        feature = Feature(f)
        models.extend(parse_feature(feature, from_crs, to_crs, progress))
    return models


def parse(input_file: str, from_crs: str = None, to_crs: str = None, progress: Progress = None):
    contents = read_json(input_file)
    return parse_data(contents, from_crs, to_crs, progress)


