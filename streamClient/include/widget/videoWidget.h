//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <decoder/IVideoSink.h>
#include <client/structs.h>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <QMutex>

class VideoWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core, public IVideoSink {
public:
    explicit VideoWidget(QWidget* parent = nullptr) : QOpenGLWidget(parent) {}
    ~VideoWidget() override;

    void onVideoFrame(const VideoFrame& videoFrame) override;

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

private:
    bool compileShader();
    void cleanup();
    void initTexture(GLuint textureHandle, const char* uniformName, uint32_t textureIndex);

    QRect updateItemRect();
    void updateMVP();
    void updateTexture(GLuint textureHandle, uint32_t textureIndex, uint32_t frameWidth, uint32_t frameHeight, int stride, char* data);

private:
    GLuint program_{};
    GLuint vao_{};
    GLuint vbo_{};
    GLuint ebo_{};
    GLuint texture_[3]{};
    GLint texUniformLocation_[3]{};
    GLint modelViewMatUniformLoc_{};
    int texWidth_[3]{};
    int texHeight_[3]{};

    VideoFrame videoFrame_{};
    QMutex mutex_;
};
