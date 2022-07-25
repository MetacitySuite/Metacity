from typing import Tuple
#from metacity.datamodel.project import Project
from metacity.geometry import LegoBuilder
from metacity.utils.filesystem import write_json
import metacity.utils.filesystem as fs
#from metacity.utils.transform import tile_coords_in_range
import os


def legofy(output_dir: str, start: Tuple[float, float], end: Tuple[float, float], coordinates_decimal_precision=2, box_filter_size_range=(5, 45), box_filter_step=5):
    """
    Generate lego tiles from a project. The lego tiles are generated in the output directory, stores json and png with heightmap.
    
    Args:
        project (str): The project to generate lego tiles from.
        output_dir (str): The directory to write the lego tiles to.
        start (Tuple[float, float]): The start coordinates of the lego tiles.
        end (Tuple[float, float]): The end coordinates of the lego tiles.
        coordinates_decimal_precision (int): The number of decimal places to round the coordinates to.
        box_filter_size_range (Tuple[int, int]): The range of box filter sizes to use.
        box_filter_step (int): The step size of the box filter.

    Returns:
        None
    """
    pass

    #fs.create_dir_if_not_exists(output_dir)
    #tiles = tile_coords_in_range(start, end, 1000)
    #builder = LegoBuilder()

    #TODO
