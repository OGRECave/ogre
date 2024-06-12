# This file is part of the OGRE project.
# It is subject to the license terms in the LICENSE file found in the top-level directory
# of this distribution and at https://www.ogre3d.org/licensing.
# SPDX-License-Identifier: MIT

import numpy as np
import Ogre
import Ogre.Numpy

from pathlib import Path
import sys

def read_splat_ply(filename):
    f = open(filename, 'rb')

    if f.readline().strip() != b"ply":
        raise ValueError("Not a ply file")

    f.readline() # endianess
    num_vertices = int(f.readline().strip().split()[2])

    # expect format as below
    NUM_PROPS = 62
    for _ in range(NUM_PROPS):
        f.readline()

    if f.readline().strip() != b"end_header":
        raise ValueError("Invalid ply file")

    data = np.frombuffer(f.read(), dtype=np.float32).reshape(num_vertices, NUM_PROPS)

    xyz = data[:, :3]
    sh = data[:, 6:54]
    opacity = data[:, 54:55]
    scale = np.exp(data[:, 55:58])
    rot = data[:, 58:62] # wxyz (same as ogre)
    # normalise quaternion
    rot = rot / np.linalg.norm(rot, axis=1)[:, None]

    return xyz, sh, opacity, scale, rot

SH_C0 = 0.28209479177387814

def sigmoid(x):
    return 1 / (1 + np.exp(-x))

def sh0_to_diffuse(sh):
    return SH_C0 * sh + 0.5

def compute_cov3d(scale, rot):
    q = Ogre.Quaternion(rot)
    m = Ogre.Matrix3()
    q.ToRotationMatrix(m)

    S = np.diag(scale)
    M = Ogre.Numpy.view(m) @ S

    Cov = M @ M.T

    return np.diag(Cov), np.array([Cov[0, 1], Cov[0, 2], Cov[1, 2]], dtype=np.float32)

def splat_to_mesh(xyz, color, covd, covu):
    Ogre.MaterialManager.getSingleton().create("pointcloud", Ogre.RGN_DEFAULT)

    mesh = Ogre.MeshManager.getSingleton().createManual("splat", Ogre.RGN_DEFAULT)
    sub = mesh.createSubMesh()
    sub.useSharedVertices = True
    sub.operationType = Ogre.RenderOperation.OT_POINT_LIST
    sub.setMaterialName("pointcloud")

    n = len(xyz)

    sub.createVertexData()
    sub.vertexData.vertexCount = n

    decl = sub.vertexData.vertexDeclaration
    hbm = Ogre.HardwareBufferManager.getSingleton()
    source = 0

    buffers = [(xyz.astype(np.half), Ogre.VET_HALF3, Ogre.VES_POSITION, 0),
               (color, Ogre.VET_UBYTE4_NORM, Ogre.VES_DIFFUSE, 0),
               (covd.astype(np.half), Ogre.VET_HALF3, Ogre.VES_TEXTURE_COORDINATES, 0),
               (covu.astype(np.half), Ogre.VET_HALF3, Ogre.VES_TEXTURE_COORDINATES, 1)]

    for data, vtype, vusage, vindex in buffers:
        decl.addElement(source, 0, vtype, vusage, vindex)
        hwbuf = hbm.createVertexBuffer(decl.getVertexSize(source), n, Ogre.HBU_CPU_ONLY)
        sub.vertexData.vertexBufferBinding.setBinding(source, hwbuf)
        hwbuf.writeData(0, hwbuf.getSizeInBytes(), data)
        source += 1

    mesh._setBounds(Ogre.AxisAlignedBox(xyz.min(axis=0), xyz.max(axis=0))) # pylint: disable=protected-access

    return mesh

def main(input_ply):
    xyz, sh, opacity, scale, rot = read_splat_ply(input_ply)

    sh0 = sh[:, :3]
    color = np.clip(np.hstack((sh0_to_diffuse(sh0), sigmoid(opacity)))*255, 0, 255).astype(np.uint8)

    N = len(xyz)

    covd = np.empty((N, 3), dtype=np.float32)
    covu = np.empty((N, 3), dtype=np.float32)
    mod = (N - 1)//100

    for i, (s, r) in enumerate(zip(scale, rot)):
        covd[i], covu[i] = compute_cov3d(s, r)
        if i % mod == 0:
            print(f"computing covariances {100*i/len(xyz):.0f}%", end="\r")

    hbm = Ogre.DefaultHardwareBufferManager()
    root = Ogre.Root("", "", "")
    mesh = splat_to_mesh(xyz, color, covd, covu)
    ser = Ogre.MeshSerializer()
    ser.exportMesh(mesh, str(Path(input_ply).with_suffix(".mesh")))
    del mesh
    del root
    del hbm

if __name__ == "__main__":
    main(sys.argv[1])
