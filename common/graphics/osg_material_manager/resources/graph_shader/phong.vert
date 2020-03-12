void phong_vert(in vec4 worldPos) {
    for (int i=0; i<numLights; ++i) {
        if (lightIsSet[i] == 1) {
            if (lightIsDirectional[i] == 1) {
                lightDir[i] = -lightPos[i];
            } else {
                lightDir[i] = worldPos.xyz-lightPos[i];
            }
        }
    }
    eyeVec = osg_ViewMatrixInverse[3].xyz-worldPos.xyz;
}