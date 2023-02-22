#pragma once

enum class PostProcessUniformType
{
	Undefined,
	Int,
	Float,
	Vec2,
	Vec3
};

struct PostProcessUniformValue
{
	PostProcessUniformType Type = PostProcessUniformType::Undefined;
	float Values[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct PostProcessShader
{
	FString Target;
	FString ShaderLumpName;
	int ShaderVersion = 0;

	FString Name;
	bool Enabled = false;

	TMap<FString, PostProcessUniformValue> Uniforms;
	TMap<FString, FString> Textures;
};

extern TArray<PostProcessShader> PostProcessShaders;

