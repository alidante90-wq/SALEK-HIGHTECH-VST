#pragma once
#include <JuceHeader.h>
#include <cmath>

/** OpenGL 3.2 shader backdrop: perspective 3D mesh grid + floating cubes. */
class OpenGLGridBackdrop : public juce::Component,
                           private juce::OpenGLRenderer,
                           private juce::Timer
{
public:
    OpenGLGridBackdrop()
    {
        openGLContext.setOpenGLVersionRequired (juce::OpenGLContext::openGL3_2);
        openGLContext.setRenderer (this);
        openGLContext.attachTo (*this);
        openGLContext.setContinuousRepainting (true);
        startTimerHz (30);
    }
    ~OpenGLGridBackdrop() override
    {
        stopTimer();
        openGLContext.detach();
    }
    void setPulse (float p) { pulse = juce::jlimit (0.f, 1.f, p); }
    void setAccent (juce::Colour c) { accent = c; }

    void newOpenGLContextCreated() override
    {
        glReady = false;
        shader.reset();
        const char* vs = R"(
            #version 150
            in vec3 position;
            uniform mat4 projectionMatrix;
            uniform mat4 viewMatrix;
            uniform float time;
            uniform float pulse;
            out float vFade;
            void main() {
                vec3 p = position;
                p.y += sin(time * 1.5 + p.x * 2.0 + p.z * 1.7) * 0.03 * (0.4 + pulse);
                vec4 world = viewMatrix * vec4(p, 1.0);
                gl_Position = projectionMatrix * world;
                vFade = clamp(1.0 - length(world.xyz) * 0.12, 0.0, 1.0);
            }
        )";
        const char* fs = R"(
            #version 150
            uniform vec3 accentColor;
            uniform float pulse;
            in float vFade;
            out vec4 fragColor;
            void main() {
                float a = vFade * (0.25 + pulse * 0.45);
                fragColor = vec4(accentColor, a);
            }
        )";
        shader = std::make_unique<juce::OpenGLShaderProgram> (openGLContext);
        if (! shader->addVertexShader (vs) || ! shader->addFragmentShader (fs) || ! shader->link())
        {
            shader.reset();
            return;
        }
        buildMesh();
        glReady = true;
    }

    void openGLContextClosing() override
    {
        shader.reset();
        mesh.release();
        glReady = false;
    }

    void renderOpenGL() override
    {
        using namespace ::juce::gl;
        juce::OpenGLHelpers::clear (juce::Colour::fromFloatRGBA (0.015f, 0.01f, 0.03f, 1.0f));
        if (! glReady || shader == nullptr) return;

        const float w = (float) juce::jmax (1, getWidth());
        const float h = (float) juce::jmax (1, getHeight());
        const float aspect = w / h;
        float fov = 55.0f * juce::MathConstants<float>::pi / 180.f;
        float nearP = 0.1f, farP = 40.0f;
        float f = 1.0f / std::tan (fov * 0.5f);
        float pm[16] = {
            f/aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, (farP+nearP)/(nearP-farP), -1,
            0, 0, (2*farP*nearP)/(nearP-farP), 0
        };

        float t = timeSec;
        float camX = std::sin (t * 0.15f) * 0.8f;
        float camY = 2.2f + pulse * 0.3f;
        float camZ = 5.5f;
        juce::Vector3D<float> eye (camX, camY, camZ);
        juce::Vector3D<float> center (0, 0, 0);
        juce::Vector3D<float> up (0, 1, 0);
        auto z = (eye - center);
        float zl = std::sqrt (z.x*z.x + z.y*z.y + z.z*z.z) + 1e-6f;
        z = juce::Vector3D<float> (z.x/zl, z.y/zl, z.z/zl);
        auto x = up ^ z;
        float xl = std::sqrt (x.x*x.x + x.y*x.y + x.z*x.z) + 1e-6f;
        x = juce::Vector3D<float> (x.x/xl, x.y/xl, x.z/xl);
        auto y = z ^ x;
        float vm[16] = {
            x.x, y.x, z.x, 0,
            x.y, y.y, z.y, 0,
            x.z, y.z, z.z, 0,
            -(x.x*eye.x + x.y*eye.y + x.z*eye.z),
            -(y.x*eye.x + y.y*eye.y + y.z*eye.z),
            -(z.x*eye.x + z.y*eye.y + z.z*eye.z),
            1
        };

        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader->use();
        {
            const GLint uProj  = shader->getUniformIDFromName ("projectionMatrix");
            const GLint uView  = shader->getUniformIDFromName ("viewMatrix");
            const GLint uTime  = shader->getUniformIDFromName ("time");
            const GLint uPulse = shader->getUniformIDFromName ("pulse");
            const GLint uAcc   = shader->getUniformIDFromName ("accentColor");
            if (uProj  >= 0) glUniformMatrix4fv (uProj, 1, GL_FALSE, pm);
            if (uView  >= 0) glUniformMatrix4fv (uView, 1, GL_FALSE, vm);
            if (uTime  >= 0) glUniform1f (uTime, timeSec);
            if (uPulse >= 0) glUniform1f (uPulse, pulse);
            if (uAcc   >= 0) glUniform3f (uAcc, accent.getFloatRed(), accent.getFloatGreen(), accent.getFloatBlue());
        }

        mesh.draw (openGLContext, GL_LINES);
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        if (! glReady)
        {
            g.setColour (accent.withAlpha (0.08f + pulse * 0.1f));
            float y0 = b.getHeight() * 0.45f;
            for (int i = 0; i < 14; ++i)
            {
                float t = (float) i / 13.f;
                float y = y0 + t * t * (b.getHeight() - y0);
                g.drawLine (0, y, b.getWidth(), y, 1.f);
            }
        }
        juce::ColourGradient vig (juce::Colours::transparentBlack, b.getCentreX(), b.getCentreY(),
                                  juce::Colours::black.withAlpha (0.55f), b.getX(), b.getY(), true);
        g.setGradientFill (vig);
        g.fillAll();
        g.setColour (accent.withAlpha (glReady ? 0.55f : 0.25f));
        g.setFont (juce::FontOptions (9.0f));
        g.drawText (glReady ? "GL MESH 3D" : "CPU GRID", b.getRight() - 72, 4, 68, 12, juce::Justification::centredRight);
    }

    void timerCallback() override
    {
        timeSec += 0.033f;
        repaint();
    }

private:
    void buildMesh()
    {
        mesh.reset();
        juce::Array<juce::Vector3D<float>> verts;
        const int N = 24;
        const float span = 8.0f;
        for (int i = -N; i <= N; ++i)
        {
            float t = (float) i / (float) N * span;
            verts.add ({ t, 0.f, -span }); verts.add ({ t, 0.f,  span });
            verts.add ({ -span, 0.f, t }); verts.add ({  span, 0.f, t });
            verts.add ({ t, 0.f, -span }); verts.add ({ t, 3.5f, -span });
        }
        for (int c = 0; c < 6; ++c)
        {
            float cx = std::sin ((float) c * 1.7f) * 3.f;
            float cz = std::cos ((float) c * 1.3f) * 3.f;
            float cy = 0.4f + (float) (c % 3) * 0.35f;
            float s = 0.25f;
            auto addEdge = [&](float x1,float y1,float z1,float x2,float y2,float z2){
                verts.add({x1,y1,z1}); verts.add({x2,y2,z2});
            };
            addEdge(cx-s,cy-s,cz-s, cx+s,cy-s,cz-s);
            addEdge(cx+s,cy-s,cz-s, cx+s,cy-s,cz+s);
            addEdge(cx+s,cy-s,cz+s, cx-s,cy-s,cz+s);
            addEdge(cx-s,cy-s,cz+s, cx-s,cy-s,cz-s);
            addEdge(cx-s,cy+s,cz-s, cx+s,cy+s,cz-s);
            addEdge(cx+s,cy+s,cz-s, cx+s,cy+s,cz+s);
            addEdge(cx+s,cy+s,cz+s, cx-s,cy+s,cz+s);
            addEdge(cx-s,cy+s,cz+s, cx-s,cy+s,cz-s);
            addEdge(cx-s,cy-s,cz-s, cx-s,cy+s,cz-s);
            addEdge(cx+s,cy-s,cz-s, cx+s,cy+s,cz-s);
            addEdge(cx+s,cy-s,cz+s, cx+s,cy+s,cz+s);
            addEdge(cx-s,cy-s,cz+s, cx-s,cy+s,cz+s);
        }
        mesh.vertices.swapWith (verts);
    }

    struct LineMesh
    {
        juce::Array<juce::Vector3D<float>> vertices;
        GLuint vbo = 0;
        void reset() { if (vbo) { juce::gl::glDeleteBuffers (1, &vbo); vbo = 0; } vertices.clear(); }
        void release() { reset(); }
        void draw (juce::OpenGLContext&, GLenum mode)
        {
            using namespace ::juce::gl;
            if (vertices.isEmpty()) return;
            if (vbo == 0)
            {
                glGenBuffers (1, &vbo);
                glBindBuffer (GL_ARRAY_BUFFER, vbo);
                glBufferData (GL_ARRAY_BUFFER, vertices.size() * (GLsizeiptr) sizeof (juce::Vector3D<float>),
                              vertices.getRawDataPointer(), GL_STATIC_DRAW);
            }
            else glBindBuffer (GL_ARRAY_BUFFER, vbo);
            glEnableVertexAttribArray (0);
            glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof (juce::Vector3D<float>), nullptr);
            glDrawArrays (mode, 0, vertices.size());
            glDisableVertexAttribArray (0);
            glBindBuffer (GL_ARRAY_BUFFER, 0);
        }
    };

    juce::OpenGLContext openGLContext;
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    LineMesh mesh;
    bool glReady = false;
    float timeSec = 0.f, pulse = 0.f;
    juce::Colour accent { 0xff00f0ff };
};
