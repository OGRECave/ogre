def pull() {
    try {
        checkout scm: [$class: 'MercurialSCM',
            source: 'https://bitbucket.org/sinbad/ogre',
            revision: 'default',
            clean: false]
    } catch(Exception e) {
        // mac runner is flaky
    } 
}

def test_android() {
     node("Ubuntu") {
        pull()
        
        stage("Android Build") {
            env.ANDROID_NDK = pwd()+"/android-ndk-r13b"
            env.ANDROID_SDK = "none"
            sh "ANDROID=TRUE cmake -P ci-build.cmake"
            sh "cmake --build ."
        }
    }   
}

def test_ios() {
     node("mac") {
        pull() 
        
        stage("ios Build") {
            sh "IOS=TRUE cmake -P ci-build.cmake"
            sh "cmake --build ."
        }
    }   
}

def test_platform(label, isstatic) {
    node(label) {
        pull()
        
        def type = isstatic == "TRUE" ? "static" : "shared"
        
        stage("${label} (${type}) Build") {
           def common = " -DOGRE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -DOGRE_STATIC=${isstatic} ."
           if(isUnix()) {
                if(label == "mac") {
                    common += " -G Xcode"
                }
               
                sh "cmake -DOGRE_BUILD_DEPENDENCIES=OFF -DCMAKE_CXX_FLAGS=-Werror ${common}"
                
                if(label == "mac") {
                    sh "cmake --build . -- ONLY_ACTIVE_ARCH=YES"
                } else {
                    sh "cmake --build ."
                }
           } else {
                def cxxflags = '-DGTEST_HAS_TR1_TUPLE=0 -EHsc'
                if(isstatic == "FALSE") {
                   cxxflags += " -WX"
                }

                bat """cmake -DOGRE_BUILD_COMPONENT_PROPERTY=FALSE -DOGRE_BUILD_RENDERSYSTEM_GL=FALSE -DCMAKE_CXX_FLAGS="${cxxflags}" ${common}"""
                bat "cmake --build ."
           }
        }
        if(label == "Ubuntu") { 
           stage("${label} (${type}) Test") {
                sh "xvfb-run -a bin/Test_Ogre"
           }
        }
    }
}

bitbucketStatusNotify ( buildState: 'INPROGRESS' )
try {
    parallel (failFast: false,
    linux_static: { test_platform("Ubuntu", "TRUE") },
    linux_dynamic: { test_platform("Ubuntu", "FALSE") },
    android: { test_android() },
    mac_dynamic: { test_platform("mac", "FALSE") },
    mac_static: { test_platform("mac", "TRUE") },
    ios: { test_ios() },
    windows_static: { test_platform("windows", "TRUE") },
    windows_dynamic: { test_platform("windows", "FALSE") }
    ) 
    bitbucketStatusNotify ( buildState: 'SUCCESSFUL' )
} catch(Exception e) {
    bitbucketStatusNotify ( buildState: 'FAILED' )
}
