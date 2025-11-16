#include "raylib.h"
#include "raymath.h"

#include <vector>

struct Plane
{
	Vector3 normal;
	float distance;
};

struct Frustum
{
	Plane planes[6]; // izquierda, derecha, abajo, arriba, near, far
};

struct MyAABB
{
	Vector3 min;
	Vector3 max;
};

struct SceneObject
{
	Model model;
	Vector3 position;
	MyAABB aabb;
	bool isVisible;
};

MyAABB CalculateLocalAABB(Mesh mesh);
MyAABB GetUpdatedAABB(MyAABB localBB, Matrix transform);
void NormalizePlane(Plane* plane);
void DrawAABB(MyAABB aabb, Color color);
bool IsAABBInFrustum(Frustum& frustum, MyAABB& aabb);
void ExtractFrustumPlanes(Frustum* frustum, Matrix viewProjection);

const int objectsQuant = 50;

void main()
{
	SceneObject objects[objectsQuant];

	Frustum cameraFrustum;

	Camera camera = { 0 };
	camera.position = { 20.0f, 15.0f, 20.0f };
	camera.target = { 0.0f, 0.0f, 0.0f };
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	float nearPlane = 0.1f;
	float farPlane = 50.0f;

	std::vector<SceneObject> sceneObjects;

	InitWindow(800, 600, "Frustum Culling");

	//objects[0].model = LoadModel("res/decahedron.obj");
	//objects[0].position = { 5.0f, 0.0f, 0.0f };
	//objects[0].isVisible = true;
	//objects[0].aabb = CalculateLocalAABB(*objects[0].model.meshes);

	//sceneObjects.push_back(objects[0]);

	//objects[1].model = LoadModel("res/dodecahedron.obj");
	//objects[1].position = { 3.0f, 2.0f, 0.0f };
	//objects[1].isVisible = true;
	//objects[1].aabb = CalculateLocalAABB(*objects[1].model.meshes);

	//sceneObjects.push_back(objects[1]);

	//objects[2].model = LoadModel("res/icosahedron.obj");
	//objects[2].position = { 5.0f, 0.0f, 1.0f };
	//objects[2].isVisible = true;
	//objects[2].aabb = CalculateLocalAABB(*objects[2].model.meshes);

	//sceneObjects.push_back(objects[2]);

	//objects[3].model = LoadModel("res/octahedron.obj");
	//objects[3].position = { 0.0f, 5.0f, 0.0f };
	//objects[3].isVisible = true;
	//objects[3].aabb = CalculateLocalAABB(*objects[3].model.meshes);

	//objects[4].model = LoadModel("res/tetrahedron.obj");
	//objects[4].position = { 0.0f, 0.0f, 0.0f };
	//objects[4].isVisible = true;
	//objects[4].aabb = CalculateLocalAABB(*objects[4].model.meshes);

	for (int i = 0; i < 50; i++)
	{
		objects[i].model = LoadModel("res/tetrahedron.obj");
		objects[i].position = { (float)(rand() % 100 - 50), 0.0f, (float)(rand() % 100 - 50)  };
		objects[i].isVisible = true;
		objects[i].aabb = CalculateLocalAABB(*objects[i].model.meshes);
		sceneObjects.push_back(objects[i]);
	}

	sceneObjects.push_back(objects[4]);

	DisableCursor();

	while (!WindowShouldClose())
	{
		UpdateCamera(&camera, CAMERA_FREE);

		float fov = camera.fovy * DEG2RAD;
		float aspectRatio = (float)GetScreenWidth() / (float)GetScreenHeight();
		Matrix view = GetCameraMatrix(camera);
		Matrix projection = MatrixPerspective(fov, aspectRatio, nearPlane, farPlane);
		Matrix viewProjection = MatrixMultiply(view, projection);

		ExtractFrustumPlanes(&cameraFrustum, viewProjection);

		for (int i = 0; i < sceneObjects.size(); i++)
		{
			Matrix transform = MatrixTranslate(sceneObjects[i].position.x, sceneObjects[i].position.y, sceneObjects[i].position.z);

			MyAABB worldAABB = GetUpdatedAABB(sceneObjects[i].aabb, transform);

			sceneObjects[i].isVisible = IsAABBInFrustum(cameraFrustum, worldAABB);
		}

		if (IsKeyPressed(KEY_P))
		{
			camera.fovy += 5.0f;
		}
		if (IsKeyPressed(KEY_L))
		{
			camera.fovy -= 5.0f;
		}

		BeginDrawing();
		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		for (int i = 0; i < sceneObjects.size(); i++)
		{
			if (sceneObjects[i].isVisible)
			{
				DrawModel(sceneObjects[i].model, sceneObjects[i].position, 1.0f, RED);
				DrawModelWires(sceneObjects[i].model, sceneObjects[i].position, 1.0f, MAROON);

				MyAABB worldAABB = GetUpdatedAABB(sceneObjects[i].aabb, MatrixTranslate(sceneObjects[i].position.x, sceneObjects[i].position.y, sceneObjects[i].position.z));
				DrawAABB(worldAABB, BLUE);
			}
			else
			{
				DrawModelWires(sceneObjects[i].model, sceneObjects[i].position, 1.0f, LIGHTGRAY);
				MyAABB worldAABB = GetUpdatedAABB(sceneObjects[i].aabb, MatrixTranslate(sceneObjects[i].position.x, sceneObjects[i].position.y, sceneObjects[i].position.z));
				DrawAABB(worldAABB, GRAY);
			}
		}

		DrawGrid(20, 1.0f);
		EndMode3D();

		int visibleCount = 0;
		for (int i = 0; i < sceneObjects.size(); i++)
		{
			if (sceneObjects[i].isVisible) visibleCount++;
		}
		DrawText(TextFormat("Visible objects: %d/%d", visibleCount, sceneObjects.size()), 10, 10, 20, BLACK);

		EndDrawing();
	}

}

void DrawAABB(MyAABB aabb, Color color)
{
	Vector3 size =
	{
		aabb.max.x - aabb.min.x,
		aabb.max.y - aabb.min.y,
		aabb.max.z - aabb.min.z
	};
	Vector3 center =
	{
		aabb.min.x + size.x * 0.5f,
		aabb.min.y + size.y * 0.5f,
		aabb.min.z + size.z * 0.5f
	};

	DrawCubeWiresV(center, size, color);
}

MyAABB CalculateLocalAABB(Mesh mesh)
{
	MyAABB aabb;
	if (mesh.vertexCount == 0)
	{
		aabb.min = Vector3Zero();
		aabb.max = Vector3Zero();
		return aabb;
	}

	aabb.min = { mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
	aabb.max = aabb.min;

	for (int i = 1; i < mesh.vertexCount; i++)
	{
		Vector3 v =
		{
			mesh.vertices[i * 3 + 0],
			mesh.vertices[i * 3 + 1],
			mesh.vertices[i * 3 + 2]
		};

		aabb.min.x = fminf(aabb.min.x, v.x);
		aabb.min.y = fminf(aabb.min.y, v.y);
		aabb.min.z = fminf(aabb.min.z, v.z);

		aabb.max.x = fmaxf(aabb.max.x, v.x);
		aabb.max.y = fmaxf(aabb.max.y, v.y);
		aabb.max.z = fmaxf(aabb.max.z, v.z);
	}
	return aabb;
}

MyAABB GetUpdatedAABB(MyAABB localBB, Matrix transform)
{
	Vector3 corners[8];
	corners[0] = { localBB.min.x, localBB.min.y, localBB.min.z };
	corners[1] = { localBB.max.x, localBB.min.y, localBB.min.z };
	corners[2] = { localBB.min.x, localBB.max.y, localBB.min.z };
	corners[3] = { localBB.min.x, localBB.min.y, localBB.max.z };
	corners[4] = { localBB.max.x, localBB.max.y, localBB.max.z };
	corners[5] = { localBB.min.x, localBB.max.y, localBB.max.z };
	corners[6] = { localBB.max.x, localBB.min.y, localBB.max.z };
	corners[7] = { localBB.max.x, localBB.max.y, localBB.min.z };

	for (int i = 0; i < 8; i++)
	{
		corners[i] = Vector3Transform(corners[i], transform);
	}

	Vector3 min = corners[0];
	Vector3 max = corners[0];
	for (int i = 1; i < 8; i++)
	{
		min.x = fminf(min.x, corners[i].x);
		min.y = fminf(min.y, corners[i].y);
		min.z = fminf(min.z, corners[i].z);

		max.x = fmaxf(max.x, corners[i].x);
		max.y = fmaxf(max.y, corners[i].y);
		max.z = fmaxf(max.z, corners[i].z);
	}

	return { min, max };
}

void NormalizePlane(Plane* plane)
{
	float length = Vector3Length(plane->normal);

	if (length == 0.0f)
	{
		return;
	}

	plane->normal.x /= length;
	plane->normal.y /= length;
	plane->normal.z /= length;
	plane->distance /= length;
}

void ExtractFrustumPlanes(Frustum* frustum, Matrix viewProjection)
{
	Matrix viewMatrix = viewProjection;

	//left
	frustum->planes[0].normal.x = viewMatrix.m3 + viewMatrix.m0;
	frustum->planes[0].normal.y = viewMatrix.m7 + viewMatrix.m4;
	frustum->planes[0].normal.z = viewMatrix.m11 + viewMatrix.m8;
	frustum->planes[0].distance = viewMatrix.m15 + viewMatrix.m12;

	//right
	frustum->planes[1].normal.x = viewMatrix.m3 - viewMatrix.m0;
	frustum->planes[1].normal.y = viewMatrix.m7 - viewMatrix.m4;
	frustum->planes[1].normal.z = viewMatrix.m11 - viewMatrix.m8;
	frustum->planes[1].distance = viewMatrix.m15 - viewMatrix.m12;

	//bottom
	frustum->planes[2].normal.x = viewMatrix.m3 + viewMatrix.m1;
	frustum->planes[2].normal.y = viewMatrix.m7 + viewMatrix.m5;
	frustum->planes[2].normal.z = viewMatrix.m11 + viewMatrix.m9;
	frustum->planes[2].distance = viewMatrix.m15 + viewMatrix.m13;

	//top
	frustum->planes[3].normal.x = viewMatrix.m3 - viewMatrix.m1;
	frustum->planes[3].normal.y = viewMatrix.m7 - viewMatrix.m5;
	frustum->planes[3].normal.z = viewMatrix.m11 - viewMatrix.m9;
	frustum->planes[3].distance = viewMatrix.m15 - viewMatrix.m13;

	//near
	frustum->planes[4].normal.x = viewMatrix.m3 + viewMatrix.m2;
	frustum->planes[4].normal.y = viewMatrix.m7 + viewMatrix.m6;
	frustum->planes[4].normal.z = viewMatrix.m11 + viewMatrix.m10;
	frustum->planes[4].distance = viewMatrix.m15 + viewMatrix.m14;

	//far
	frustum->planes[5].normal.x = viewMatrix.m3 - viewMatrix.m2;
	frustum->planes[5].normal.y = viewMatrix.m7 - viewMatrix.m6;
	frustum->planes[5].normal.z = viewMatrix.m11 - viewMatrix.m10;
	frustum->planes[5].distance = viewMatrix.m15 - viewMatrix.m14;

	for (int i = 0; i < 6; i++)
	{
		NormalizePlane(&frustum->planes[i]);
	}
}

bool IsAABBInFrustum(Frustum& frustum, MyAABB& aabb)
{
	for (int i = 0; i < 6; i++)
	{
		Plane& plane = frustum.planes[i];

		Vector3 positiveVertex = aabb.min;
		if (plane.normal.x >= 0)
		{
			positiveVertex.x = aabb.max.x;
		}

		if (plane.normal.y >= 0)
		{
			positiveVertex.y = aabb.max.y;
		}
		if (plane.normal.z >= 0)
		{
			positiveVertex.z = aabb.max.z;
		}

		if (Vector3DotProduct(plane.normal, positiveVertex) + plane.distance < 0)
		{
			return false;
		}
	}

	return true;
}