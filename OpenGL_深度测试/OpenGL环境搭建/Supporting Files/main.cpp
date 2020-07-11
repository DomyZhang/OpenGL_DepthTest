

#include "GLShaderManager.h"
#include "GLTools.h"
#include <glut/glut.h>

#include "GLFrustum.h"
#include "GLFrame.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"

GLBatch triangleBatch;

GLShaderManager shaderManager;

// 设置角色帧，作为相机
GLFrame  viewFrame;// 观察者位置
GLFrustum           viewFrustum;// 透视投影 - GLFrustum类
GLTriangleBatch     torusBatch;
// 两个矩阵
GLMatrixStack       modelViewMatix;// 模型矩阵
GLMatrixStack       projectionMatrix;// 透视矩阵
//
GLGeometryTransform transformPipeline;// 几何转换


// 定义字段判断 正背面剔除/深度测试 开启与否
int iCull = 0;
int iDepth = 0;


// 右键菜单栏选项 开启关闭深度测试
void ProcessMenu(int value) {
    
    switch(value)
    {
        case 1:
            iCull = !iCull;
            break;

        case 2:
            iDepth = !iDepth;
            break;
    }

    glutPostRedisplay();
}

// 键盘上下左右 事件
// 观察者移动 物体不动
void SpecialKeys(int key, int x, int  y) {
    
    if(key == GLUT_KEY_UP)
        viewFrame.RotateWorld(m3dDegToRad(-5.0), 1.0f, 0.0f, 0.0f);
    
    if(key == GLUT_KEY_DOWN)
        viewFrame.RotateWorld(m3dDegToRad(5.0), 1.0f, 0.0f, 0.0f);
    
    if(key == GLUT_KEY_LEFT)
        viewFrame.RotateWorld(m3dDegToRad(-5.0), 0.0f, 1.0f, 0.0f);
    
    if(key == GLUT_KEY_RIGHT)
        viewFrame.RotateWorld(m3dDegToRad(5.0), 0.0f, 1.0f, 0.0f);
    
    //重新刷新window
    glutPostRedisplay();
}

// 初始化 设置
void SetupRC() {
    
    // ================ 简单绘制一个立体的圈圈 =================
    // 设置背景颜色
     glClearColor(0.0f, 0.3f, 0.3f, 1.0f );
     
     //初始化着色器管理器
     shaderManager.InitializeStockShaders();
     
     //将相机向后移动7个单元：肉眼到物体之间的距离
     viewFrame.MoveForward(7.0);
     
     //创建一个甜甜圈
     //void gltMakeTorus(GLTriangleBatch& torusBatch, GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor);
     //参数1：GLTriangleBatch 容器帮助类
     //参数2：外边缘半径
     //参数3：内边缘半径
     //参数4、5：主半径和从半径的细分单元数量
     gltMakeTorus(torusBatch, 1.0f, 0.3f, 52, 26);
    //点的大小
     glPointSize(4.0f);
    
    
    
    /* ================ 简单绘制一个三角形 =================
    //设置背影颜色
    glClearColor(0.0f,0.0f,1.0f,1.0f);
    
    //初始化着色管理器
    shaderManager.InitializeStockShaders();
    
    //设置三角形，其中数组vVert包含所有3个顶点的x,y,笛卡尔坐标对。
    GLfloat vVerts[] = {
        
        -0.5f,0.0f,0.0f,
        0.5f,0.0f,0.0f,
        0.0f,0.5f,0.0f,
    };
    
    //批次处理
    triangleBatch.Begin(GL_TRIANGLES,3);
    
    triangleBatch.CopyVertexData3f(vVerts);
    
    triangleBatch.End();
    ================ 简单绘制一个三角形 ====================== */
}


// 视口  窗口大小改变时接受新的宽度和高度，其中0,0代表窗口中视口的左下角坐标，w，h代表像素
void ChangeSize(int w,int h) {
    // 防止h变为0
    if(h == 0)
        h = 1;
    
    // 设置视口窗口尺寸
    glViewport(0, 0, w, h);
    
    // setPerspective 函数的参数是一个从顶点方向看去的视场角度（用角度值表示）
    // 设置透视模式，初始化其透视矩阵
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    
    //4.把 透视矩阵 加载到 透视矩阵对阵中
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    //5.初始化渲染管线
    transformPipeline.SetMatrixStacks(modelViewMatix, projectionMatrix);
}

// 渲染
void RenderScene(void) {
    
    //清除窗口和深度缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    // iCull 判断是否开启背面剔除 GL_CULL_FACE
    if(iCull)
    {
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
    }
    else
        glDisable(GL_CULL_FACE);
    
    // iDepth 判断是否开启深度测试 GL_DEPTH_TEST
    if(iDepth)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    
    //把摄像机矩阵压入模型矩阵中 入栈
    modelViewMatix.PushMatrix(viewFrame);
    
    GLfloat vRed[] = { 0.0f, 0.5f, 0.5f, 1.0f };// 画笔颜色
    
    //使用默认光源着色器
    //通过光源、阴影效果跟提现立体效果
    //参数1：GLT_SHADER_DEFAULT_LIGHT 默认光源着色器
    //参数2：模型视图矩阵
    //参数3：投影矩阵
    //参数4：基本颜色值
    shaderManager.UseStockShader(GLT_SHADER_DEFAULT_LIGHT, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vRed);
    
    //绘制
    torusBatch.Draw();

    //出栈
    modelViewMatix.PopMatrix();
    glutSwapBuffers();
    
    
    
    
    
    
    /* ================ 简单绘制一个三角形 ======================
    //清除一个或一组特定的缓冲区
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    
    //设置一组浮点数来表示红色
    GLfloat vRed[] = {1.0f,0.0f,0.0f,1.0f};
    
    //传递到存储着色器，即GLT_SHADER_IDENTITY着色器，这个着色器只是使用指定颜色以默认笛卡尔坐标第在屏幕上渲染几何图形
    shaderManager.UseStockShader(GLT_SHADER_IDENTITY,vRed);
    
    //提交着色器
    triangleBatch.Draw();
    
    //将在后台缓冲区进行渲染，然后在结束时交换到前台
    glutSwapBuffers();
    ================ 简单绘制一个三角形 ====================== */
}

int main(int argc,char* argv[]) {
    
    //设置当前工作目录，针对MAC OS X
    
    gltSetWorkingDirectory(argv[0]);
    
    //初始化GLUT库
    
    glutInit(&argc, argv);
    /* 初始化双缓冲窗口，其中标志GLUT_DOUBLE、GLUT_RGBA、GLUT_DEPTH、GLUT_STENCIL分别指
        双缓冲窗口、RGBA颜色模式、深度测试、模板缓冲区
     */
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
    
    //GLUT窗口大小，标题窗口
    glutInitWindowSize(800,600);
    glutCreateWindow("Triangle");
    
    //注册回调函数
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);// 注册键盘特殊键位(上下左右键) 处理点击事件
    
    // 创建右键 Menu
    glutCreateMenu(ProcessMenu);// 右键菜单栏选项 开启关闭深度测试
    glutAddMenuEntry("Toggle cull backface",1);// 正背面剔除
    glutAddMenuEntry("Toggle depth test",2);// 深度测试的开启与否

    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    
    
    //驱动程序的初始化中没有出现任何问题。
    GLenum err = glewInit();
    if(GLEW_OK != err) {
        
        fprintf(stderr,"glew error:%s\n",glewGetErrorString(err));
        return 1;
    }
    
    //调用SetupRC
    SetupRC();
    
    glutMainLoop();
    
    return 0;
}

