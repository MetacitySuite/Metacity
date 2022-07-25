import fiona
import geopandas
from metacity.utils import filesystem as fs
from metacity.io.geojson import parse_data as parse_geojson
from metacity.geometry import Progress

__all__ = ["parse"]


def parse(shp_file: str, from_crs: str = None, to_crs: str = None, progress: Progress = None):
    """
    Parse a SHP file. All contents are transformed into Metacity objects, and returned as a list.

    Args:
        shp_file (str): Path to the SHP file.
        from_crs (str): Optional. The CRS of the input file.
        to_crs (str): Optional. The CRS to transform the input file to.
    
    Returns:
        list: List of Metacity objects.

    See Also:
            :func:`metacity.io.parse' to see other formats.

    """
    file = geopandas.read_file(shp_file)
    data = file._to_geo()
    return parse_geojson(data, from_crs, to_crs, progress)

