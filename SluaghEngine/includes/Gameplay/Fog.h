#ifndef _SE_GAMEPLAY_FOG_H_
#define _SE_GAMEPLAY_FOG_H_


#include <memory.h>


namespace SE
{
	namespace Gameplay
	{
		class Fog
		{
		private:
			struct Vertex
			{
				float position[3];
				float uv[2];
				float opacity;

				Vertex() {};
				Vertex(float x, float y, float z, float u, float v, float o = 1)
				{
					position[0] = x; position[1] = y; position[2] = z;
					uv[0] = u; uv[1] = v;
					opacity = o;
				}
			};

			struct Quad
			{
				enum class Vertices { TopLeft, TopRight, BottomLeft, BottomRight };

			private:
				Vertex vertices[6];

			public:
				Quad() {};
				void SetVertex(Vertices vertex, float x, float y, float z, float u, float v, float opacity = 1)
				{
					if (vertex == Vertices::TopLeft)
					{
						vertices[0] = Vertex(x, y, z, u, v, opacity);
						vertices[3] = Vertex(x, y, z, u, v, opacity);
					}
					else if (vertex == Vertices::TopRight)
					{
						vertices[1] = Vertex(x, y, z, u, v, opacity);
					}
					else if (vertex == Vertices::BottomLeft)
					{
						vertices[5] = Vertex(x, y, z, u, v, opacity);
					}
					else if (vertex == Vertices::BottomRight)
					{
						vertices[2] = Vertex(x, y, z, u, v, opacity);
						vertices[4] = Vertex(x, y, z, u, v, opacity);
					}
				}
			};

			struct Plane
			{
			private:
				Quad quads[27 * 27];
				bool quads_used[27 * 27] = {};

			public:
				Plane() {};
				Quad& GetQuad(unsigned int column, unsigned int row)
				{
					quads_used[row * 27 + column] = true;
					return quads[row * 27 + column];
				}
				bool GetQuadStatus(unsigned int column, unsigned int row)
				{
					return quads_used[row * 27 + column];
				}
				unsigned int GetQuadCount()
				{
					unsigned int usedQuadCount = 0;
					for (int quadI = 0; quadI < 27 * 27; quadI++)
					{
						if (quads_used[quadI])
							usedQuadCount++;
					}
					return usedQuadCount;
				}
				void GetVertexBuffer(Vertex *pVertexBuffer)
				{
					unsigned int vertexI = 0;
					for (int quadI = 0; quadI < 27 * 27; quadI++)
					{
						if (quads_used[quadI])
						{
							memcpy(pVertexBuffer + vertexI, &quads[quadI], 2 * 3 * sizeof(Vertex));

							vertexI += 2 * 3;
						}
					}
				}
			};

		private:
			static const char id_Props = 137;
			static const char id_Torch = 203;
			static const char id_Floor = 0;
			static const char id_DeadArea = 76;
			static const char id_Door1 = 22;
			static const char id_Door2 = 48;
			static const char id_Wall = 255;
			static const char id_Pillar = 225;
			static const char id_Bush = 13;

			const float fogHeight = 3.1f;
			const float fogPadding = 10.f;
			const float TEMP_uvCoord = 0;

			unsigned int fogRjHandle = -1;
			char tileValues[25][25];

			Plane plane;

		public:
			void Set(bool status);
			void SetTileValues(char tileValues[25][25]);

		private:
			void CreateFogPlane();
			void CheckAdjacentTiles(unsigned int column, unsigned int row);
		};
	}
}


#endif
