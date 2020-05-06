# Simple-DX11-Renderer

A very simple DX11 renderer that I wrote over shutdown.
I put it here in case some one needs one as an example, though it's far from perfect. Added comments in code to make it more readable.
 
Loads and compiles all shaders at runtime for simplicity. Should be NSIGHT friendly and if it complains about debug layer pass in '-nd'  flag to disable it.
 
# License
License is MIT even for well model. Textures used are voronoi generated and baked in Blender.
HDRI is originally from HDRI Haven. Then blurred and used for IBL (Not as actual IBL, just approximated so there would be something to sample from :p ).
 
# Sample
![image](https://github.com/ike-0/simple-dx11-renderer/blob/master/well6.png)
