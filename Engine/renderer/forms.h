#ifndef FORMS_H
#define FORMS_H

#include <cge_export.h>

#include <memory>
#include <vector>

#include "gfxtypes.h"
#include "../math/vector.h"

namespace CGE{

namespace Gfx{
class Buffer;
class DrawContext;
class RenderDevice;
}

//! Standard shapes: quads, lines, cylinders, boxes
/*! They're drawn through a Gfx::DrawContext on a render device. The 2D
    shapes (quads, lines) use the built-in Textured shader if the context
    has a texture, the Color shader otherwise, so the context's colour tints
    them. The 3D shapes use the Lit shader. */
class CGE_API Forms{
public:
  //! a vertex buffer, an index buffer and the parts to draw
  struct CGE_API Shape{
    struct Part{
      Gfx::PrimitiveType primitive;
      int firstIndex;
      int indexCount;
    };
    Shape();
    ~Shape();
    //! vertices in Forms::getVertexLayout
    std::unique_ptr<Gfx::Buffer> vertices;
    //! 16-bit indices
    std::unique_ptr<Gfx::Buffer> indices;
    std::vector<Part> parts;
  };

  //! the shapes on a render device
  explicit Forms(Gfx::RenderDevice& device);
  ~Forms();

  //! the layout of the shapes' vertices: position, normal, colour (white) and texture coordinates
  static const Gfx::VertexLayout& getVertexLayout();

  //! the unit quad, from -0.5 to 0.5 on each edge, in the context's model matrix
  /*! v = 0 is the top edge in a projection whose y points up, so textures
      appear upright there. */
  void drawQuad(Gfx::DrawContext& ctx) const;
  //! a quad at position spanning size, rotated by rotation degrees clockwise
  void drawQuad(Gfx::DrawContext& ctx, const Vec2f& position, const Vec2f& size, float rotation = 0) const;
  //! a line of the line width (setLineWidth)
  void drawLine(Gfx::DrawContext& ctx, const Vec2f& from, const Vec2f& to) const;
  //! a cylinder of radius 1 and height 1 around the origin along the y axis
  void drawCylinder(Gfx::DrawContext& ctx) const;
  //! a cylinder around the origin along the y axis
  static std::unique_ptr<Shape> createCylinder(Gfx::RenderDevice& device, float radius, float height, int segments);
  //! a box around the origin
  static std::unique_ptr<Shape> createBox(Gfx::RenderDevice& device, float width, float height, float depth);
  //! draws a shape with the Lit shader
  static void drawShape(Gfx::DrawContext& ctx, const Shape& shape);

  void setLineWidth(float width) { mLineWidth = width; }
private:
  //! the device's quad and cylinder
  void createShapes(Gfx::RenderDevice& device);
  float mLineWidth;
  std::unique_ptr<Shape> mQuad;
  std::unique_ptr<Shape> mCylinder;
};

}

#endif
