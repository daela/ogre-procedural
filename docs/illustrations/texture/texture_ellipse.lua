buffer = Procedural.TextureBuffer(128)
Procedural.Solid(buffer):setColour(Procedural.ColourValue(0.0, 0.5, 1.0, 1.0)):process()
Procedural.EllipseTexture(buffer):setColour(Procedural.ColourValue_Red):setRadiusX(0.4):setRadiusY(0.2):process()
tests:addTextureBuffer(buffer)
dotfile = tests:getDotFile("texture_33", "Ellipse_Demo")
dotfile:set("Solid", "texture_solid", "Ellipse", "texture_ellipse")
