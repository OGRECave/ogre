import logging
log = logging.getLogger(__name__)

import sys
from pprint import pprint

import Ogre, OgreBites, OgreRTShader

from PySide2 import QtGui as qg, QtCore as qc, QtWidgets as qw
from PySide2.QtCore import Qt

class OgreWidget(qw.QOpenGLWidget):
    """
    A work-in-progress example of how to embed Ogre into a PySide2/Qt5 application.
    """

    def __init__(self, parent: qw.QWidget=None):
        qw.QOpenGLWidget.__init__(self, parent=parent)
        self._root = Ogre.Root()
        
        # Ensure we are using the basic OpenGL renderer
        for renderer in self._root.getAvailableRenderers():
            if renderer.getName() == 'OpenGL Rendering Subsystem':
                break
        else:
            raise ValueError('OpenGL Rendering System not available to Ogre')

        # Note: British spellings on names in Ogre
        self._root.setRenderSystem(renderer)
        self._root.initialise(False)

        # Will be created in `initializeGL`
        self._rend: 'Ogre' = None

        # It does not seem to be necessary so far to run a QTimer to refresh 
        # the frame constantly. Just call `self.update()` from any method that 
        # should require a frame update

                                                
    def initializeGL(self):
        windowId = self.parent().winId()
        # print('Window ID: ', windowId)
        params = Ogre.NameValuePairList()
        params['parentWindowHandle'] = str(windowId)

        # TODO: consider making the render window the screen shape so that it 
        # can grow and shrink as needed.
        self._rend = self._root.createRenderWindow(str(windowId),
                                                   self.width(), 
                                                   self.height(), 
                                                   False, 
                                                   params)
        self._rend.setActive(True)
        self._rend.setVisible(True)

        ogreWindowId = self._rend.getCustomAttribute('WINDOW')
        # print('OgreWinId: ', ogreWindowId)
        self.setAttribute(Qt.WA_OpaquePaintEvent)

        self.locateResources()
        self.setupScene()

        # Definitely do not call the Ogre event loop, use the Qt event loop
        # instead.
        # self._root.startRendering()

    def resizeGL(self, width: int, height: int):
        self._rend.reposition(self.x(), self.y())
        # Looks like you can shrink it but not make the Window bigger than its initialized size
        self._rend.resize(width, height)
        self._rend.windowMovedOrResized()

    def paintGL(self):
        self._root.renderOneFrame()

    def locateResources(self):
        resManager = Ogre.ResourceGroupManager.getSingleton()

        # Add the Media meshes to the path
        resManager.addResourceLocation('./models', 'FileSystem', Ogre.RGN_DEFAULT)

        # Some advice on loading RTShaderLib:
        # https://forums.ogre3d.org/viewtopic.php?t=82632
        # First add shaders
        resManager.addResourceLocation('./RTShaderLib/GLSL', 'FileSystem', 'GLSL', True)
        resManager.addResourceLocation('./RTShaderLib/HLSL_Cg', 'FileSystem', 'HLSL_Cg', True)
        resManager.initialiseResourceGroup('GLSL')
        resManager.initialiseResourceGroup('HLSL_Cg')
        # Then add materials
        resManager.addResourceLocation('./RTShaderLib/materials', 'FileSystem', 'Materials', True)
        resManager.initialiseResourceGroup('Materials')
        resManager.initialiseAllResourceGroups()


    def setupScene(self):
        # Step 1: https://ogrecave.github.io/ogre/api/latest/setup.html
        sceneManager = self._root.createSceneManager()
        
        # In Python, whenever you see `getSingleton()` you need to call 
        # `.initialize()` first. Note that here it's with a 'z', in other 
        # cases its spelled with an 's'.
        OgreRTShader.ShaderGenerator.initialize()
        shaderGen = OgreRTShader.ShaderGenerator.getSingleton()
        shaderGen.addSceneManager(sceneManager)

        rootNode = sceneManager.getRootSceneNode()

        light = sceneManager.createLight('MainLight')
        lightNode = rootNode.createChildSceneNode()
        lightNode.setPosition(0, 100, 200)
        lightNode.attachObject(light)

        self.camNode = rootNode.createChildSceneNode()
        self.camNode.setPosition(0, 0, 200)
        self.camNode.lookAt(Ogre.Vector3(0, 0, -1), Ogre.Node.TS_PARENT)

        camera = sceneManager.createCamera('My Camera')
        camera.setNearClipDistance(5)
        camera.setAutoAspectRatio(True)
        self.camNode.attachObject(camera)

        viewport = self._rend.addViewport(camera)
        viewport.setBackgroundColour(Ogre.ColourValue(.1, .1, .1))
        
        athena = sceneManager.createEntity('athene.mesh')
        athenaNode = rootNode.createChildSceneNode()
        athenaNode.attachObject(athena)
        
    def mousePressEvent(self, event: qg.QMouseEvent) -> None:
        button = event.button()
        log.info(f'Pressed button {button}')
        self.setFocus(qc.Qt.MouseFocusReason)

    def mouseReleaseEvent(self, event: qg.QMouseEvent) -> None:
        button = event.button()
        log.info(f'Released button {button}')
        
    def mouseEnterEvent(self, event: qg.QMouseEvent) -> None:
        self.setFocus(qc.Qt.MouseFocusReason)

    def mouseMoveEvent(self, event: qg.QMouseEvent):
        self.setFocus(qc.Qt.MouseFocusReason)

    def wheelEvent(self, event: qg.QMouseEvent) -> None:
        if event.delta() > 0:
            self.shiftCamera(0.0, 0.0, -2.5)
        else:
            self.shiftCamera(0.0, 0.0, 2.5)

    def shiftCamera(self, dx: float, dy: float, dz: float) -> None:
        """
        Translates the camera.
        """
        print(f'Translate camera by: {dx, dy, dz}')
        self.camNode.translate(dx, dy, dz)
        self.update()

class EmbedWindow(qw.QMainWindow):
    """
    """

    def __init__(self):
        qw.QMainWindow.__init__(self)
        self.setupUi()

    def setupUi(self):
        self.setWindowTitle('Embedded Ogre3D in PySide2')

        splitter = qw.QSplitter(parent=self)
        self.setCentralWidget(splitter)

        # Menubar
        # =======
        menubar = qw.QMenuBar(self)
        self.setMenuBar(menubar)

        menuFile = qw.QMenu(menubar)
        menuFile.setTitle('File')
        actionQuit = qw.QAction(self)
        actionQuit.setText('Quit')
        actionQuit.triggered.connect(self.quitApp)
        menuFile.addAction(actionQuit)

        menuHelp = qw.QMenu(menubar)
        menuHelp.setTitle('Help')

        menubar.addAction(menuFile.menuAction())
        menubar.addAction(menuHelp.menuAction())

        # Status Bar
        # ==========
        self.statusbar = qw.QStatusBar(parent=self)
        self.setStatusBar(self.statusbar)

        statusWidget = qw.QWidget(parent=self)
        statusLayout = qw.QHBoxLayout(statusWidget)
        statusLayout.setContentsMargins(5, 0, 5, 3)
        self.statusbar.addPermanentWidget(statusWidget)

        self.progressbar  = qw.QProgressBar(parent=self)
        self.progressbar.setGeometry(30, 40, 100, 25)
        self.progressbar.setFormat('%p %')
        self.progressbar.setMinimum(0)
        self.progressbar.setMaximum(100)
        self.progressbar.reset()
        statusLayout.addStretch()
        statusLayout.addWidget(self.progressbar)


        # Sidebar
        # =======
        sidebar = qw.QScrollArea(parent=self)
        sidebar.setMinimumWidth(256)
        spMinimumExpanding = qw.QSizePolicy(qw.QSizePolicy.Minimum, qw.QSizePolicy.Expanding)
        sidebar.setSizePolicy(spMinimumExpanding)
        sidebar.setMaximumWidth(512)
        splitter.addWidget(sidebar)

        laySide = qw.QVBoxLayout()
        laySide.setContentsMargins(2, 2, 2, 2)
        laySide.setSpacing(3)
        sidebar.setLayout(laySide)

        pbHello = qw.QPushButton('Hello', parent=sidebar)
        laySide.addWidget(pbHello)

        sbTest = qw.QSpinBox(parent=sidebar)
        laySide.addWidget(sbTest)

        # Ogre
        # ====
        self.view = OgreWidget(parent=self)
        spExpanding = qw.QSizePolicy(qw.QSizePolicy.Expanding, qw.QSizePolicy.Expanding)
        self.view.setSizePolicy(spExpanding)
        splitter.addWidget(self.view)

        self.resize(1920, 1080)

    def quitApp(self):
        log.info('Shutting down...')
        # Ogre seems to shut itself down nicely without any effort on our part
        del self.view
        Q_APP.exit() 
        

if __name__ == '__main__':
    Q_APP = qw.QApplication(sys.argv)
    mainWindow = EmbedWindow()
    mainWindow.show()
    Q_APP.exec_()