#include <iostream>
#include <filesystem>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

namespace fs = std::filesystem;
struct MyTraits : public OpenMesh::DefaultTraits
{
	typedef OpenMesh::Vec3d Point; // use double-values points
};
typedef OpenMesh::PolyMesh_ArrayKernelT<MyTraits>  MyMesh;

void ListDir(const std::string& dir, const std::string& extension, std::vector<std::string>& filelist)
{
	for (const auto& entry : fs::directory_iterator(dir))
	{
		fs::path fPath = entry.path();
		if (fs::is_directory(fPath))
		{
			ListDir(fPath.string(), extension, filelist);
		}
		if (fs::is_regular_file(fPath))
		{
			if (fPath.has_extension())
			{
				if (fPath.extension().string().compare(extension) == 0)
				{
					filelist.emplace_back(fPath.string());
				}
			}
		}
	}
}

int main(int argc, char** argv)
{
	std::vector<std::string> filelist;
	ListDir(argv[1], ".obj", filelist);

	std::vector<MyMesh::Point> main_points;
	std::vector<std::vector<size_t>> main_faces;

	int last_vert_num = 0;
	for (const auto& mesh_name : filelist)
	{
		std::cout << mesh_name << std::endl;

		MyMesh sub_mesh;
		OpenMesh::IO::read_mesh(sub_mesh, mesh_name);

		for (const auto& v_h : sub_mesh.vertices())
		{
			main_points.emplace_back(sub_mesh.point(v_h));
		}

		for (const auto& f_h : sub_mesh.faces())
		{
			std::vector<size_t> vertex_ids;
			for (const auto& fv_h : f_h.vertices())
			{
				vertex_ids.emplace_back(fv_h.idx() + last_vert_num);
			}
			main_faces.emplace_back(vertex_ids);
		}

		last_vert_num = main_points.size();
	}


	MyMesh main_mesh;

	// generate vertices
	std::vector<MyMesh::VertexHandle> vhandles;
	vhandles.reserve(main_points.size());
	for (const auto& main_pt : main_points)
	{
		vhandles.emplace_back(main_mesh.add_vertex(main_pt));
	}

	// generate faces
	std::vector<MyMesh::VertexHandle>  face_vhandles;
	for (const auto& main_face : main_faces)
	{
		face_vhandles.clear();
		for (const auto& main_vid : main_face)
		{
			face_vhandles.push_back(vhandles[main_vid]);

		}
		main_mesh.add_face(face_vhandles);
	}


	if (!OpenMesh::IO::write_mesh(main_mesh, argv[2]))
	{
		std::cerr << "Cannot write mesh !" << std::endl;
		return 1;
	}

	return 0;
}