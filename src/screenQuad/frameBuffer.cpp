

// FrameBuffer::FrameBuffer()
// {
//     shader->
//     createDepthFramebuffer();
// }

// void createDepthFramebuffer(void) 
// {
//     glGenFramebuffers(1, &framebufferID);
//     glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

//     // create a color attachment texture
//     glGenTextures(1, &bufferTexture);
//     glBindTexture(GL_TEXTURE_2D, bufferTexture);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
//     glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTexture, 0);

//     // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
//     // unsigned int rbo;
//     // glGenRenderbuffers(1, &rbo);
//     // glBindRenderbuffer(GL_RENDERBUFFER, rbo);
//     // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
//     // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 

//     // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
//     if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//         throw "Framebuffer not initialized correctly!";
//     }
//     glBindFramebuffer(GL_FRAMEBUFFER, 0);
// }