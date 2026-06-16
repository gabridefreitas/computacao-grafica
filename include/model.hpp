#ifndef MODEL_HPP_
#define MODEL_HPP_

#include <algorithm>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <mesh.hpp>
#include <material.hpp>
#include <light.hpp>
#include <curve.hpp>
#include <optional>

#include <stb_image.h>

enum Axis {
	X_AXIS, Y_AXIS, Z_AXIS
};

enum Operation {
	TRANSLATE,
	ROTATE,
	SCALE,
	SCALE_SIMETRICAL
};

class Model {
public:
	Material material;
	bool isLight = false;
	int currentMaterial=1;
	std::optional<Curves> curves = std::nullopt;

	Model(
			std::string path,
			std::optional<Curves> curves = std::nullopt):curves(curves){
		loadModel(path);
	}

	Model(
			std::string path,
			glm::vec3 translate,
			glm::vec3 rotate,
			glm::vec3 scale,
			std::optional<Curves> curves = std::nullopt) :
		translate(translate),
		rotate(rotate),
		scale(scale),
		curves(curves)
	{
		loadModel(path);
	}

	Model(
			std::string path,
			glm::vec3 lightPos){
		translate = lightPos;
		scale = glm::vec3(0.2);
		this->isLight = true;
		loadModel(path);
	}
	void Draw(Shader& shader, float delta){
		shader.setMatrix4f("model", getModel(delta));
		shader.setMaterial(material);
		for (size_t i = 0; meshes.size() > i; i++)
			meshes[i].Draw(shader);
	}
	void DrawLight(Shader& shader, glm::vec3 position){
		translate = position;
		shader.setMatrix4f("model", getModel(0));
		for (size_t i = 0; meshes.size() > i; i++)
			meshes[i].Draw(shader);
	}

	void cycleMaterial(bool up) {
		if (up){
			currentMaterial = std::min(++currentMaterial, static_cast<int>(MATERIALS.size()) - 1);
			material = MATERIALS[currentMaterial];
		}else{
			currentMaterial = std::max(--currentMaterial, 0);
			material = MATERIALS[currentMaterial];
		}
	}

	void setOperation(Operation operation){
		this->operation = operation;
	}

	void moveModel(Axis axis, float deltaTime){
		glm::vec3* chosen_op = nullptr;
		float sensitivity = getSensitivity() * deltaTime;
		switch (operation) {
			case TRANSLATE:
				chosen_op = &translate; break;
			case ROTATE:
				chosen_op = &rotate; break;
			case SCALE:
				chosen_op = &scale; break;
			case SCALE_SIMETRICAL:
				scale += sensitivity;
				return;
		};
		switch (axis) {
			case X_AXIS:
				chosen_op->x += sensitivity; break;
			case Y_AXIS:
				chosen_op->y += sensitivity; break;
			case Z_AXIS:
				chosen_op->z += sensitivity; break;
		}
	}

	void resetModelScaleAndRotation() {
		rotate = glm::vec3(0.0f);
		scale = glm::vec3(1.0f);
	}

private:
	glm::vec3 translate = glm::vec3(0.0f);
	glm::vec3 rotate = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	Operation operation = TRANSLATE;

	glm::mat4 getModel(float delta) {
		glm::vec3 curvePos(0.0f);
		if (curves.has_value()){ 
			curves->update(delta);
			curvePos = curves->getPosition();
		}

		glm::mat4 model(1.0f);
		model = glm::translate(model, translate + curvePos);
		model = glm::rotate(model, glm::radians(rotate.x), glm::vec3(1.0, 0.0, 0.0));
		model = glm::rotate(model, glm::radians(rotate.y), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model, glm::radians(rotate.z), glm::vec3(0.0, 0.0, 1.0));
		model = glm::scale(model, scale);
		return model;
	}

	float getSensitivity() {
		float sens;
		switch (operation) {
			case TRANSLATE: sens = 5.0f; break;
			case ROTATE: sens = 20.0f; break;
			case SCALE:
			case SCALE_SIMETRICAL: 
									 sens = 3.0f; break;
		}
		return sens;
	}

	std::vector<Mesh> meshes;
	std::string directory;
	std::vector<Texture> textures_loaded;

	void loadModel(std::string path){
		Assimp::Importer import;
		const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);	

		if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
		{
			std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode *node, const aiScene *scene){
		// process all the node's meshes (if any)
		for(unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
			meshes.push_back(processMesh(mesh, scene));			
		}
		// then do the same for each of its children
		for(unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene){
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		for (size_t i = 0; i < mesh->mNumVertices; i++){
			glm::vec2 texCoords(0.0f);
			if (mesh->mTextureCoords[0]) {
				texCoords.x = mesh->mTextureCoords[0][i].x;
				texCoords.y = mesh->mTextureCoords[0][i].y;
			}
			Vertex vertex{
				glm::vec3(
						mesh->mVertices[i].x,
						mesh->mVertices[i].y,
						mesh->mVertices[i].z),
					glm::vec3(
							mesh->mNormals[i].x,
							mesh->mNormals[i].y,
							mesh->mNormals[i].z),
					texCoords
			};
			vertices.push_back(vertex);
		}

		for(size_t i = 0; i < mesh->mNumFaces; i++){
			aiFace face = mesh->mFaces[i];
			for (size_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if(mesh->mMaterialIndex >= 0)
		{
			aiColor3D diffuse(0,0,0);
			aiColor3D specular(0,0,0);
			aiColor3D ambient(0,0,0);
			float shininess;
			
			aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture> diffuseMaps = loadMaterialTextures(material, 
					aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			std::vector<Texture> specularMaps = loadMaterialTextures(material, 
					aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			
			material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
			material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
			material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
			material->Get(AI_MATKEY_SHININESS, shininess);
			setMaterialFromAiColor3D(ambient, diffuse, specular, shininess);
		
		} 
		return Mesh(vertices, indices, textures);
	}

	std::vector<Texture> loadMaterialTextures(
			aiMaterial *mat,
			aiTextureType type,
			std::string typeName){
		std::vector<Texture> textures;
		for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			bool skip = false;
			for(unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true; 
					break;
				}
			}
			if(!skip)
			{   // if texture hasn't been loaded already, load it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture); // add to loaded textures
			}
		}
		return textures; 
	}
	unsigned int TextureFromFile(const char *path, const std::string &directory)
	{
		std::string filename = std::string(path);
		filename = directory + '/' + filename;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}
	void setMaterialFromAiColor3D(
			aiColor3D ambient, aiColor3D diffuse, aiColor3D specular, float shininess){
		this->material.ambient.x = ambient.r;
		this->material.ambient.y = ambient.g;
		this->material.ambient.z = ambient.b;
		this->material.diffuse.x = diffuse.r;
		this->material.diffuse.y = diffuse.g;
		this->material.diffuse.z = diffuse.b;
		this->material.specular.x = specular.r;
		this->material.specular.y = specular.g;
		this->material.specular.z = specular.b;
		this->material.shininess = shininess;
	}

};

#endif
