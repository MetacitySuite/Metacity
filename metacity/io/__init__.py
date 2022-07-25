from metacity.io.geojson import parse as parse_geojson
from metacity.io.shapefile import parse as parse_shapefile
import metacity.utils.filesystem as fs
from metacity.geometry import Progress 

__all__ = ["parse", "parse_recursively"]

def parse(file: str, from_crs: str = None, to_crs: str = None,  progress: Progress = None):
    if file.endswith('.shp'):
        return parse_shapefile(file, from_crs, to_crs, progress)
    elif file.endswith('.json'):
        return parse_geojson(file, from_crs, to_crs, progress)


def parse_recursively(directory: str, from_crs: str = None, to_crs: str = None):
    models = []
    progress = Progress("Loading Models")
    for file in fs.list_files_recursive(directory):
        data = parse(file, progress)
        if data is not None:
            models.extend(parse(file, from_crs, to_crs))
    return models

