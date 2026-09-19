//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#include <widget/videoWidget.h>

#include <QFile>
#include <QMutexLocker>
#include <QMatrix4x4>
#include <QPainter>

namespace {
    const double EPSILON = 1e-8;
    const char* textureUniformName[3] = { "textureY", "textureU", "textureV" };

    std::string readShaderFile(const QString& filePath) {
        QFile file(filePath);

        // 尝试以只读和文本模式打开文件
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << QString("error: cannot open file: %1 error info: %2").arg(filePath, file.errorString());
            return {};
        }

        QTextStream in(&file);
        const QString content = in.readAll();
        file.close();

        return content.toStdString();
    }
}

VideoWidget::~VideoWidget() {
    makeCurrent();
    cleanup();
    doneCurrent();
}

void VideoWidget::setSelected(bool selected) {
    if (selected_ == selected) {
        return;
    }
    selected_ = selected;
    update();
}

bool VideoWidget::isSelected() const {
    return selected_;
}

void VideoWidget::onVideoFrame(const VideoFrame& videoFrame) {
    {
        QMutexLocker locker(&mutex_);
        videoFrame_ = videoFrame;
    }

    QMetaObject::invokeMethod(this, [this] {
        update();
    }, Qt::QueuedConnection);
}

void VideoWidget::initializeGL() {
    initializeOpenGLFunctions();

    if (!compileShader()) {
        return;
    }

    const float vertices[] = {
        // 位置              // 纹理坐标
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  // 左下
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,  // 右下
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,  // 右上
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f   // 左上
    };

    const unsigned int indices[] = {
        0, 1, 2,  // 第一个三角形
        2, 3, 0   // 第二个三角形
    };

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenTextures(3, texture_);
    for (uint32_t i = 0; i < 3; ++i) {
        initTexture(texture_[i], textureUniformName[i], i);
    }

    modelViewMatUniformLoc_ = glGetUniformLocation(program_, "modelViewMatrix");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void VideoWidget::resizeGL(int width, int height) {
    QOpenGLWidget::resizeGL(width, height);
}

void VideoWidget::paintGL() {
    QPainter painter(this);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program_);

    updateMVP();

    {
        QMutexLocker locker(&mutex_);

        if (videoFrame_.data[0]) {
            for (uint32_t i = 0; i < 3; ++i) {
                uint32_t textureHandle = texture_[i];
                updateTexture(textureHandle, i, videoFrame_.frameWidth, videoFrame_.frameHeight,
                    videoFrame_.stride[i], videoFrame_.data[i]);
            }
        }
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glUseProgram(0);

    drawSelectionBorder(painter);
    painter.end();
}

void VideoWidget::drawSelectionBorder(QPainter& painter) const {
    QPen pen(selected_ ? QColor(0x00, 0xFF, 0xCC) : QColor(0x23, 0x23, 0x32));
    pen.setWidth(2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect().adjusted(1, 1, -1, -1));
}

bool VideoWidget::compileShader() {
    // 顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderStr = readShaderFile(":/shader/video.vert");
    const char* vertexShaderSource = vertexShaderStr.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512]{};
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        qWarning() << "vertex shader compile fail:" << infoLog;
        return false;
    }

    // 片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderStr = readShaderFile(":/shader/video.frag");
    const char* fragmentShaderSource = fragmentShaderStr.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        qWarning() << "fragment shader compile fail:" << infoLog;
        return false;
    }

    // 着色器程序
    program_ = glCreateProgram();
    glAttachShader(program_, vertexShader);
    glAttachShader(program_, fragmentShader);
    glLinkProgram(program_);

    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program_, 512, nullptr, infoLog);
        qWarning() << "shader link compile fail:" << infoLog;
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

void VideoWidget::cleanup() {
    glDeleteProgram(program_);
    program_ = 0;

    glDeleteVertexArrays(1, &vao_);
    vao_ = 0;

    glDeleteBuffers(1, &vbo_);
    vbo_ = 0;

    glDeleteBuffers(1, &ebo_);
    ebo_ = 0;

    glDeleteTextures(3, texture_);
}

void VideoWidget::initTexture(GLuint textureHandle, const char* uniformName, uint32_t textureIndex) {
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    texUniformLocation_[textureIndex] = glGetUniformLocation(program_, uniformName);
    glBindTexture(GL_TEXTURE_2D, 0);
}

QRect VideoWidget::updateItemRect() {
    QMutexLocker locker(&mutex_);

    const uint32_t frameHeight = videoFrame_.frameHeight;
    const uint32_t frameWidth = videoFrame_.frameWidth;
    const uint32_t widgetWidth = width();
    const uint32_t widgetHeight = height();

    double aspectRatio = (double)frameWidth / frameHeight;
    if (videoFrame_.picWidthHeightRatio > EPSILON) {
        aspectRatio *= videoFrame_.picWidthHeightRatio;
    }

    uint32_t rectHeight = widgetHeight;
    uint32_t rectWidth = (uint32_t)std::round((float)rectHeight * aspectRatio) & ~1;
    if (rectWidth > widgetWidth) {
        rectWidth = widgetWidth;
        rectHeight = (uint32_t)std::round((float)rectWidth / aspectRatio) & ~1;
    }

    QRect itemRect{};
    itemRect.setX((widgetWidth - rectWidth) / 2);
    itemRect.setY((widgetHeight - rectHeight) / 2);
    itemRect.setWidth(rectWidth);
    itemRect.setHeight(rectHeight);

    return itemRect;
}

void VideoWidget::updateMVP() {
    const QRect itemRect = updateItemRect();
    const float left = itemRect.left();
    const float right = left + itemRect.width();
    const float top = itemRect.top();
    const float bottom = top + itemRect.height();

    const float vertices[] = {
        // 位置                     // 纹理坐标
        left, bottom, 0.0f,        0.0f, 0.0f,  // 左下
        right, bottom, 0.0f,       1.0f, 0.0f,  // 右下
        right, top, 0.0f,          1.0f, 1.0f,  // 右上
        left, top, 0.0f,           0.0f, 1.0f   // 左上
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(vao_);

    QMatrix4x4 modelViewMatrix{};
    modelViewMatrix.ortho(0, width(), 0, height(), -1, 1);

    glUniformMatrix4fv(modelViewMatUniformLoc_, 1, GL_FALSE, modelViewMatrix.constData());
}

void VideoWidget::updateTexture(GLuint textureHandle, uint32_t textureIndex, uint32_t frameWidth, uint32_t frameHeight,
                                int stride, char* data) {
    if (!data || !frameWidth || !frameHeight || stride <= 0) {
        return;
    }

    const int width = (textureIndex == 0) ? (int)frameWidth : (int)(frameWidth / 2);
    const int height = (textureIndex == 0) ? (int)frameHeight : (int)(frameHeight / 2);
    if (width <= 0 || height <= 0) {
        return;
    }

    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glUniform1i(texUniformLocation_[textureIndex], (int)textureIndex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, std::max(stride, width));
    if (width != texWidth_[textureIndex] || height != texHeight_[textureIndex]) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        texWidth_[textureIndex] = width;
        texHeight_[textureIndex] = height;
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, data);
    }
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}
