from metacity.io.geojson import parse as parse_geojson
from metacity.io.shapefile import parse as parse_shp
from metacity.io import parse_recursively


def test_geojson(geojson_dataset: str):
    objects = parse_geojson(geojson_dataset)
    assert len(objects) == 24

def test_geojson_transform(geojson_dataset: str):
    objects = parse_geojson(geojson_dataset, 'EPSG:3857', 'EPSG:5514')
    assert len(objects) == 24

def test_shp_line(shp_line_dataset: str):
    objects = parse_shp(shp_line_dataset)
    assert len(objects) == 5041

def test_shp_line_transform(shp_line_dataset: str):
    objects = parse_shp(shp_line_dataset, 'EPSG:5514', 'EPSG:3857')
    assert len(objects) == 5041

def test_shp_poly(shp_poly_dataset: str):
    objects = parse_shp(shp_poly_dataset)
    assert len(objects) == 2826

def test_shp_poly_transform(shp_poly_dataset: str):
    objects = parse_shp(shp_poly_dataset, 'EPSG:5514', 'EPSG:3857')
    assert len(objects) == 2826

def test_recursive_parse(data_directory):
    objects = [ o for o in parse_recursively(data_directory) ]
    assert len(objects) == 2826 + 5041 + 24
    


