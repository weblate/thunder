import qbs

Project {
    id: tests
    property stringList srcFiles: [
        "tests.cpp",
        "../**/tst_*.h",
        "../**/tst_*.cpp"
    ]

    property stringList incPaths: [
        "../thirdparty/unittestpp",
        "../thirdparty/next/tests",
        "../thirdparty/next/inc",
        "../thirdparty/next/inc/core",
        "../thirdparty/next/inc/math",
        "../thirdparty/next/inc/anim",
        "../engine/includes",
    ]

    QtApplication {
        name: "tests"
        condition: tests.desktop
        files: tests.srcFiles
        excludeFiles: [
            "../Tools/**/*.*",
            "../qt/**/*.*",
            "../**/qbs/**/*.*"
        ]
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "glfw-editor" }
        Depends { name: "freetype-editor" }
        Depends { name: "zlib-editor" }
        Depends { name: "physfs-editor" }

        Depends { name: "Qt"; submodules: ["core", "test"] }

        bundle.isBundle: false

        cpp.defines: ["NEXT_SHARED"]
        cpp.includePaths: tests.incPaths

        property string prefix: qbs.targetOS.contains("windows") ? "lib" : ""
        cpp.dynamicLibraries: [ ]
        cpp.cxxLanguageVersion: "c++14"

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@rpath"
            cpp.rpaths: "@executable_path/../Frameworks/"
        }

        Properties {
            condition: qbs.targetOS[0] === "windows"
            cpp.dynamicLibraries: [ "Shell32", "User32", "Gdi32", "Advapi32", "opengl32" ]
        }
        Properties {
            condition: qbs.targetOS[0] === "linux"
            cpp.dynamicLibraries: [ "X11", "Xrandr", "Xi", "Xxf86vm", "Xcursor", "Xinerama", "dl", "pthread" ]
        }
        Properties {
            condition: qbs.targetOS[0] === "macos"
            cpp.weakFrameworks: [ "OpenGL", "Cocoa", "CoreVideo", "IOKit" ]
        }

        Group {
            name: "Install " + tests.tests_NAME
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: tests.BIN_PATH + "/" + tests.bundle
            qbs.installPrefix: tests.PREFIX
        }
    }
}
