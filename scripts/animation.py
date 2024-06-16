import os
import cv2
import math
import time
from os.path import isfile, join

exec_path = "/Users/abiri/goinfre/RT/rt"
save_path = "/Users/abiri/goinfre/RT/scripts/"
keep_frames = True
frame_count = 12
frames_per_second = 24
resolution = 720
rendered_files = []

###################################################
#            SCENE GENERATION FUNCTION            #
###################################################

def generate_rotary_camera(index, center=(0, 0, 0), radius=5.0, rotation=0.5, rotation_offset=0, y=0):
    scene = '''<scene ambiant="0.8" AA="1" resolution="%d" light_samples="1" refraction_depth="1" reflection_depth="1"/>
    <camera position="(%f, %f, %f)" lookat="(%f, %f, %f)" fov="40"/>
    <light center="(5, 100, 110)" intensity="0.5" color="#FFFFFF"/>
    <fractal center="(0, 1, 0)" color="#FF00FF" iterations="50" steps="50" power="3"/>
    <plane center="(0, 0, 0)" normal="(0, 1, 0)" color="#FFFFFF" reflection="0.1"/>\n'''
    angle = ((index / frame_count) * (rotation * 2.0 * math.pi)) - rotation_offset * 2.0 * math.pi
    newx = center[0] + radius * math.cos(angle)
    newz = center[2] + radius * math.sin(angle)
    return (scene % (resolution, newx, y, newz, center[0], center[1], center[2]))

def generate_evolutionary_fractal_camera(index, center=(0, 0, 0), radius=5.0, rotation=0.5, rotation_offset=0, y=0, min_steps=0, max_steps=50):
    scene = '''<scene ambiant="0.7" AA="4" resolution="%d" light_samples="1" refraction_depth="1" reflection_depth="1"/>
    <camera position="(%f, %f, %f)" lookat="(%f, %f, %f)" fov="40"/>
    <fractal center="(1, 1, 1)" color="#FF00FF" iterations="200" steps="%d" power="3"/>\n'''
    step = min_steps + (index / frame_count) * (max_steps - min_steps)
    angle = ((index / frame_count) * (rotation * 2.0 * math.pi)) - rotation_offset * 2.0 * math.pi
    radius += (index/frame_count) * 4
    newx = center[0] + radius * math.cos(angle)
    newz = center[2] + radius * math.sin(angle)
    return (scene % (resolution, newx, y, newz, center[0], center[1], center[2], 50))

def generate_spiral_camera(index, center=(0, 0, 0), radius=5.0, rotation=0.5, rotation_offset=0, y=0, height=50):
    scene = '''<scene ambiant="0.7" AA="1" resolution="%d" light_samples="1" refraction_depth="1" reflection_depth="1"/>
    <camera position="(%f, %f, %f)" lookat="(%f, %f, %f)" fov="40"/>
    <light center="(0, 10.18, 18.79)"  radius="1" intensity="0.7" color="#FFFFFF"/>
    <light center="(-25, 25, -25)"  radius="1" intensity="0.7" color="#FFFFFF"/>
    <fractal center="(0, 0, 0)" color="#FF00FF" iterations="100" steps="200" power="3"/>
    <torus center="(5, 18, 37)" big_radius="10" small_radius="1" color="(255,127,80)"/>
    <cube reflection="0" center="(32,43,57)" color="(255,127,80)" side="3"/>
    <cone length="30" center="(0, 10, 0)" color="#FF00FF" axis="(0, 1, 0)" radius="6"/>
    <sphere center="(5, 10, 50)" color="#FF0000" radius="3"/>
    <ellipsoid transparency="0.7" refraction="1.1" center="(10, 30, 45)" axis="(1,5, 5)" translation="(3, 0, 0)" color="#000000" radius="4"/>
    <plane reflection="0.1" center="(0, 0, 0)" length="(160, 160)"  U="(0, 0, 1)" V="(1, 0, 0)"  color="#D3D3D3" ttexture_mapping=":checker" mapping_position="(0, 0, 0.25)"/>
    <plane reflection="0.1" center="(0, 50, 0)" length="(160, 160)"  U="(0, 0, 1)" V="(1, 0, 0)"  color="#D3D3D3" ttexture_mapping=":pastel" mapping_position="(0, 0, 0.25)"/>
    <plane reflection="0.1" center="(50, 0, 0)" length="(160, 160)"  U="(0, 0, 1)" V="(0, 1, 0)"  color="#D3D3D3" ttexture_mapping=":pastel" mapping_position="(0, 0, 0.25)"/>
    <plane reflection="0.1" center="(-50, 0, 0)" length="(160, 160)"  U="(0, 0, 1)" V="(0, 1, 0)"  color="#D3D3D3" ttexture_mapping=":pastel" mapping_position="(0, 0, 0.25)"/>
    <plane reflection="0.1" center="(0, 0, 120)" length="(160, 160)"  U="(1, 0, 0)" V="(0, 1, 0)"  color="#D3D3D3" ttexture_mapping=":pastel" mapping_position="(0, 0, 0.25)"/>
    <plane reflection="0.1" center="(0, 0, -100)" length="(160, 160)"  U="(1, 0, 0)" V="(0, 1, 0)"  color="#D3D3D3" ttexture_mapping=":pastel" mapping_position="(0, 0, 0.25)"/>'''
    angle = ((index / frame_count) * (rotation * 2.0 * math.pi)) - rotation_offset * 2.0 * math.pi
    x = center[0] + radius * math.cos(angle)
    y = center[1] + height * angle / (2 * math.pi)  # Adjust y-coordinate for spiral height
    z = center[2] + radius * math.sin(angle)
    print("POS", x, y, z)
    print("Looking at", *center)
    return (scene % (resolution, x, y, z, center[0], center[1], center[2]))

###################################################
#               BUILTIN FUNCTIONS                 #
###################################################

def render_current_scene(scene, index):
    with open(join(save_path, "autorender_scene.xml"), 'w+') as scene_file:
        scene_file.write(scene)
        scene_file.close()
        image_filename = join(save_path, "render_" + str(index) + ".bmp")
        print ("rendering frame : %d" % (index,))
#        print("PATH :", exec_path + " --no_window " + image_filename + " " + join(save_path, "autorender_scene.xml"))
        exit_status = os.system(exec_path + " --no_window " + image_filename + " " + join(save_path, "autorender_scene.xml"))
        if exit_status != 0:
            print("Rendering frame %d failed with exit code : %d" % (index, exit_status))
            exit(1)
        rendered_files.append(image_filename)
        return
    print("Cannot save temp scene file to " + save_path)
    exit(1)

def cleanup_saved_files():
    for filename in rendered_files:
        os.remove(filename)

def convert_frames_to_video():
    print("MERGING INTO VIDEO")
    frame_array = []
    for i in range(len(rendered_files)):
        filename = rendered_files[i]
        img = cv2.imread(filename)
        height, width, layers = img.shape
        size = (width,height)
        frame_array.append(img)
    out = cv2.VideoWriter(join(save_path, "output" + str(time.time()) + ".mp4"), cv2.VideoWriter_fourcc(*'MJPG'), frames_per_second, size)
    for i in range(len(frame_array)):
        cv2.imshow('frame', frame_array[i])
        out.write(frame_array[i])
    out.release()
    print("VIDEO CREATED")

def main():
    if exec_path == None:
        print ("Please enter exec path")
        exit(1)
    if save_path == None:
        print ("Please enter save path")
        exit(1)
    for index in range(frame_count):
        # SCENE GENERATION FUNCTION v
        #scene = generate_rotary_camera(index, center=(0, 1, 0), radius=30, rotation=0.25, rotation_offset=0.2)
        #scene = generate_spiral_camera(index, center=(0, 15.18, 18.79), radius=30, rotation=0.25, rotation_offset=0.2)
        scene = generate_evolutionary_fractal_camera(index, center=(1, 1, 1), radius=0.3, rotation=1, rotation_offset=0, min_steps=0, max_steps=50)
        # SCENE GENERATION FUNCTION ^
        render_current_scene(scene, index)
    convert_frames_to_video()
    if not keep_frames:
        cleanup_saved_files()

if __name__ == "__main__":
    main()
