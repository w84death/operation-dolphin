#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../include/model.h"

// STB_Image implementation is included in main.c, so we just access its functions
extern unsigned char* stbi_load(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels);
extern unsigned char* stbi_load_from_memory(const unsigned char* buffer, int len, int* x, int* y, int* channels_in_file, int desired_channels);
extern void stbi_image_free(void* retval_from_stbi_load);

// Load a GLB model using Assimp
Model loadModel(const char* filepath) {
    Model model = {0};
    
    // Import the model using Assimp with more processing flags for better material handling
    const struct aiScene* scene = aiImportFile(filepath, 
        aiProcess_Triangulate |  
        aiProcess_FlipUVs | 
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_GenUVCoords |
        aiProcess_TransformUVCoords);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("Error loading model: %s\n", aiGetErrorString());
        return model;
    }
    
    // Allocate memory for model data
    model.num_meshes = scene->mNumMeshes;
    model.num_vertices = (unsigned int*)malloc(model.num_meshes * sizeof(unsigned int));
    model.num_indices = (unsigned int*)malloc(model.num_meshes * sizeof(unsigned int));
    model.vertices = (float**)malloc(model.num_meshes * sizeof(float*));
    model.indices = (unsigned int**)malloc(model.num_meshes * sizeof(unsigned int*));
    model.texture_ids = (unsigned int*)malloc(model.num_meshes * sizeof(unsigned int));
    
    // Process embedded textures first
    model.num_textures = scene->mNumMaterials;
    model.textures = (Texture*)malloc(model.num_textures * sizeof(Texture));
    
    // First, check if there are embedded textures in the GLB file
    for (unsigned int i = 0; i < scene->mNumTextures; i++) {
        // Just checking for existence here
        struct aiTexture* tex = scene->mTextures[i];
        (void)tex; // To avoid unused variable warning
    }
    
    // Load material textures with enhanced color information
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        struct aiMaterial* material = scene->mMaterials[i];
        model.textures[i].id = 0;
        model.textures[i].path = NULL;
        model.textures[i].data = NULL;
        
        // Generate texture
        glGenTextures(1, &model.textures[i].id);
        glBindTexture(GL_TEXTURE_2D, model.textures[i].id);
        
        // Check for diffuse texture
        struct aiString texPath;
        if (aiGetMaterialString(material, AI_MATKEY_TEXTURE_DIFFUSE(0), &texPath) == AI_SUCCESS) {
            // Check if this is an embedded texture (starts with *)
            if (texPath.data[0] == '*') {
                int texIndex = atoi(texPath.data + 1);
                if (texIndex >= 0 && (unsigned)texIndex < scene->mNumTextures) {
                    struct aiTexture* tex = scene->mTextures[texIndex];
                    if (tex->mHeight == 0) {
                        int width, height, channels;
                        unsigned char* imageData = stbi_load_from_memory((unsigned char*)tex->pcData, tex->mWidth, &width, &height, &channels, 4);
                        if (imageData) {
                            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                            stbi_image_free(imageData);
                        } else {
                            printf("Failed to load compressed texture with stb_image.\n");
                        }
                    } else {
                        // Uncompressed RGBA texture
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->mWidth, tex->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->pcData);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    }
                }
            }
        }

        // Check for diffuse color
        struct aiColor4D diffuse;
        if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == AI_SUCCESS) {
            // We can use this diffuse color data later if needed
        }
    }
    
    // Process each mesh
    for (unsigned int i = 0; i < model.num_meshes; i++) {
        struct aiMesh* mesh = scene->mMeshes[i];
        
        // Get material index for this mesh
        unsigned int materialIndex = mesh->mMaterialIndex;
        model.texture_ids[i] = model.textures[materialIndex].id;
        
        // Get material color for this mesh
        struct aiMaterial* material = scene->mMaterials[materialIndex];
        struct aiColor4D diffuse = {0.8f, 0.8f, 0.8f, 1.0f}; // Default light gray
        
        // Try to get diffuse color from material
        if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse) != AI_SUCCESS) {
            // If no diffuse color found, use a color based on mesh index
            switch (i % 5) {
                case 0: diffuse.r = 0.9f; diffuse.g = 0.2f; diffuse.b = 0.2f; break; // Red
                case 1: diffuse.r = 0.2f; diffuse.g = 0.8f; diffuse.b = 0.2f; break; // Green
                case 2: diffuse.r = 0.2f; diffuse.g = 0.2f; diffuse.b = 0.9f; break; // Blue
                case 3: diffuse.r = 0.9f; diffuse.g = 0.9f; diffuse.b = 0.2f; break; // Yellow
                case 4: diffuse.r = 0.8f; diffuse.g = 0.2f; diffuse.b = 0.8f; break; // Purple
            }
        }
        
        model.num_vertices[i] = mesh->mNumVertices;
        // Format: position(3) + normal(3) + texcoord(2) + color(4)
        model.vertices[i] = (float*)malloc(mesh->mNumVertices * 12 * sizeof(float));
        
        // Process vertices
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            // Position
            model.vertices[i][j*12] = mesh->mVertices[j].x;
            model.vertices[i][j*12+1] = mesh->mVertices[j].y;
            model.vertices[i][j*12+2] = mesh->mVertices[j].z;
            
            // Normal
            if (mesh->mNormals) {
                model.vertices[i][j*12+3] = mesh->mNormals[j].x;
                model.vertices[i][j*12+4] = mesh->mNormals[j].y;
                model.vertices[i][j*12+5] = mesh->mNormals[j].z;
            } else {
                model.vertices[i][j*12+3] = 0.0f;
                model.vertices[i][j*12+4] = 1.0f;
                model.vertices[i][j*12+5] = 0.0f;
            }
            
            // Texture coordinates
            if (mesh->mTextureCoords[0]) {
                model.vertices[i][j*12+6] = mesh->mTextureCoords[0][j].x;
                model.vertices[i][j*12+7] = mesh->mTextureCoords[0][j].y;
            } else {
                model.vertices[i][j*12+6] = 0.0f;
                model.vertices[i][j*12+7] = 0.0f;
            }
            
            // Color (from material)
            model.vertices[i][j*12+8] = diffuse.r;
            model.vertices[i][j*12+9] = diffuse.g;
            model.vertices[i][j*12+10] = diffuse.b;
            model.vertices[i][j*12+11] = diffuse.a;
        }
        
        // Process indices
        model.num_indices[i] = mesh->mNumFaces * 3; // triangles only
        model.indices[i] = (unsigned int*)malloc(model.num_indices[i] * sizeof(unsigned int));
        
        unsigned int index_offset = 0;
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            struct aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                model.indices[i][index_offset++] = face.mIndices[k];
            }
        }
    }
    
    // Clean up Assimp's resources
    aiReleaseImport(scene);
    
    return model;
}

// Free the allocated model data
void freeModel(Model model) {
    for (unsigned int i = 0; i < model.num_meshes; i++) {
        free(model.vertices[i]);
        free(model.indices[i]);
    }
    
    // Delete textures
    if (model.textures) {
        for (unsigned int i = 0; i < model.num_textures; i++) {
            if (model.textures[i].id != 0) {
                glDeleteTextures(1, &model.textures[i].id);
            }
            free(model.textures[i].path);
        }
        free(model.textures);
    }
    
    free(model.num_vertices);
    free(model.num_indices);
    free(model.vertices);
    free(model.indices);
    free(model.texture_ids);
}

// Load texture from file
GLuint loadTextureFromFile(const char* filename) {
    GLuint textureID;
    int width, height, channels;
    
    // Load the image using stb_image
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        printf("Failed to load terrain texture: %s\n", filename);
        return 0;
    }
    
    // Generate texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Set texture parameters for trilinear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Use linear filtering for magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Use trilinear filtering for minification
    
    // Determine format based on channels
    GLenum format = GL_RGB;
    GLint internalFormat = GL_RGB8; // Request specific internal format
    if (channels == 4) {
        format = GL_RGBA;
        internalFormat = GL_RGBA8;
    } else if (channels == 1) {
        format = GL_RED; // Or GL_LUMINANCE if targeting older GL versions
        internalFormat = GL_R8; // Or GL_LUMINANCE8
    } else if (channels != 3) {
        // If not 1, 3, or 4 channels, force load as RGB
        stbi_image_free(data);
        data = stbi_load(filename, &width, &height, &channels, 3); // Force 3 channels (RGB)
        if (data) {
            format = GL_RGB;
            internalFormat = GL_RGB8;
            channels = 3;
        } else {
             printf("Failed to reload texture with forced channels: %s\n", filename);
             glBindTexture(GL_TEXTURE_2D, 0); // Unbind
             glDeleteTextures(1, &textureID); // Delete texture object
             stbi_image_free(data); // Free data if reload failed
             return 0;
        }
    }
    
    // Upload texture data and generate Mipmaps using GLU
    gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, width, height, format, GL_UNSIGNED_BYTE, data);
    
    // Free image data
    stbi_image_free(data);
    
    return textureID;
}