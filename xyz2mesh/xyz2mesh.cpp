// xyz2mesh.cpp

#define CGAL_EIGEN3_ENABLED

#include <CGAL/Timer.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>

#include <CGAL/trace.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/IO/output_surface_facets_to_polyhedron.h>
#include <CGAL/IO/read_xyz_points.h>

#include <CGAL/Poisson_reconstruction_function.h>
#include <CGAL/Point_with_normal_3.h>
#include <CGAL/property_map.h>

#include <CGAL/compute_average_spacing.h>

#include <CGAL/jet_smooth_point_set.h>
#include <CGAL/grid_simplify_point_set.h>

#include <CGAL/mst_orient_normals.h>
#include <CGAL/pca_estimate_normals.h>
#include <CGAL/property_map.h>
#include <CGAL/remove_outliers.h>

#include <fstream>
#include <iostream>
#include <deque>
#include <cstdlib>
#include <math.h>

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------

// Kernel
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;

// Simple geometric types
typedef Kernel::FT                          FT;
typedef Kernel::Point_3                     Point;
typedef Kernel::Vector_3                    Vector;
typedef CGAL::Point_with_normal_3<Kernel>   Point_with_normal;
typedef Kernel::Sphere_3                    Sphere;

typedef std::vector<Point>                  PointList;
typedef std::pair<Point, Vector>            PointVectorPair;
typedef std::vector<PointVectorPair>        PointVectorList;
typedef std::vector<Point_with_normal>      PointWNList;

// polyhedron
typedef CGAL::Polyhedron_3<Kernel>          Polyhedron;

// Poisson implicit function
typedef CGAL::Poisson_reconstruction_function<Kernel> Poisson_reconstruction_function;

// Surface mesher
typedef CGAL::Surface_mesh_default_triangulation_3                          STr;
typedef CGAL::Surface_mesh_complex_2_in_triangulation_3<STr>                C2t3;
typedef CGAL::Implicit_surface_3<Kernel, Poisson_reconstruction_function>   Surface;

struct Counter {
    int i, N;
    Counter(int N): i(0), N(N){}
    
    void operator()(){
        i++;
        if(i == N){
            std::cerr << "Counter reached " << N << std::endl;
        }
    }
};

struct InsertVisitor {
    Counter& c;
    InsertVisitor(Counter& c):c(c){}
    void before_insertion(){c();}
};

void simplifyCloud(PointList& points, float cell_size){
    // simplification by clustering using erase-remove idiom
    points.erase(CGAL::grid_simplify_point_set(points.begin(), points.end(), cell_size),
                 points.end());

    // Optional: after erase(), use Scott Meyer's "swap trick" to trim excess capacity
    std::vector<Point>(points).swap(points);
}

void removeOutliers(PointList& points, float removed_percentage, int nb_neighbors){
    // Removes outliers using erase-remove idiom.
    // The Identity_property_map property map can be omitted here as it is the default value.
    // removed_percentage = 5.0; // percentage of points to remove
    // nb_neighbors = 24; // considers 24 nearest neighbor points
    points.erase(CGAL::remove_outliers(points.begin(), points.end(),
                                       CGAL::Identity_property_map<Point>(),
                                       nb_neighbors, removed_percentage),
                 points.end());
    
    // Optional: after erase(), use Scott Meyer's "swap trick" to trim excess capacity
    std::vector<Point>(points).swap(points);
}

void estimateNormals(const PointList& points, PointVectorList& point_vectors, int nb_neighbors){
    point_vectors.resize(points.size());
    for (int i=0; i<points.size(); i++) {
        point_vectors[i].first = points[i];
    }
    
    // Estimates normals direction.
    // Note: pca_estimate_normals() requires an iterator over points
    // as well as property maps to access each point's position and normal.
    // nb_neighbors = 18; // K-nearest neighbors = 3 rings
    CGAL::pca_estimate_normals(point_vectors.begin(), point_vectors.end(),
                               CGAL::First_of_pair_property_map<PointVectorPair>(),
                               CGAL::Second_of_pair_property_map<PointVectorPair>(),
                               nb_neighbors);
}

void orientNormals(PointVectorList& points, int nb_neighbors, bool trim){
    // Orients normals.
    // Note: mst_orient_normals() requires an iterator over points
    // as well as property maps to access each point's position and normal.
    std::vector<PointVectorPair>::iterator unoriented_points_begin =
    CGAL::mst_orient_normals(points.begin(), points.end(),
                             CGAL::First_of_pair_property_map<PointVectorPair>(),
                             CGAL::Second_of_pair_property_map<PointVectorPair>(),
                             nb_neighbors);
    
    // Optional: delete points with an unoriented normal
    // if you plan to call a reconstruction algorithm that expects oriented normals.
    if (trim)
        points.erase(unoriented_points_begin, points.end());
}

inline void convert(const PointVectorList& point_vectors, PointWNList& point_with_normals){
    int n = point_vectors.size();
    point_with_normals.resize(n);
    for (int i=0; i<n; i++) {
        point_with_normals[i].position() = point_vectors[i].first;
        point_with_normals[i].normal() = point_vectors[i].second;
    }
}

struct Vertex{
    Vertex(long double _x, long double _y, long double _z):x(_x),y(_y),z(_z){};
    long double x,y,z;
};

void savePly(const Polyhedron& _poly, std::string _path, bool _useBinary = false ) {
    
    //  COPY DATA (vertex + index)
    //
    std::vector<Vertex> vertices;
    std::map<Point, uint16_t> vertices_indices;
    std::vector<uint16_t> indices;
    int count = 0;
    for (auto it=_poly.vertices_begin(); it!=_poly.vertices_end(); ++it) {
        auto& p = it->point();
        vertices.push_back(Vertex(p.x(), p.y(), p.z()));
        vertices_indices[p] = count++;
    }

    for (auto it=_poly.facets_begin(); it!=_poly.facets_end(); ++it) {
        indices.push_back(vertices_indices[it->halfedge()->vertex()->point()]);
        indices.push_back(vertices_indices[it->halfedge()->next()->vertex()->point()]);
        indices.push_back(vertices_indices[it->halfedge()->prev()->vertex()->point()]);
    }

    //  CREATE PLY  
    //
    std::ios_base::openmode binary_mode = _useBinary ? std::ios::binary : (std::ios_base::openmode)0;
    std::fstream os(_path.c_str(), std::ios::out | binary_mode);
    
    //  Header
    os << "ply" << std::endl;
    if(_useBinary) {
        os << "format binary_little_endian 1.0" << std::endl;
    } else {
        os << "format ascii 1.0" << std::endl;
    }
    
    if(vertices.size()>0){
        os << "element vertex " << vertices.size() << std::endl;
        os << "property float x" << std::endl;
        os << "property float y" << std::endl;
        os << "property float z" << std::endl;
    }
    
    unsigned char faceSize = 3;
    if(indices.size()>0){
        os << "element face " << indices.size() / faceSize << std::endl;
        os << "property list uchar int vertex_indices" << std::endl;
    } else {
        os << "element face " << vertices.size() / faceSize << std::endl;
        os << "property list uchar int vertex_indices" << std::endl;
    }

    os << "end_header" << std::endl;
    
    // Vertices
    for(int i = 0; i < vertices.size(); i++){
        if(_useBinary) {
            os.write((char*) &vertices[i].x, sizeof(Vertex));
        } else {
            os << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z;
        }
        if(!_useBinary) {
            os << std::endl;
        }
    }
    
    // Indices
    if(indices.size()>0) {
        for(int i = 0; i < indices.size(); i += faceSize) {
            if(_useBinary) {
                os.write((char*) &faceSize, sizeof(unsigned char));
                for(int j = 0; j < faceSize; j++) {
                    int curIndex = indices[i + j];
                    os.write((char*) &curIndex, sizeof(uint16_t));
                }
            } else {
                os << (int) faceSize << " " << indices[i] << " " << indices[i+1] << " " << indices[i+2] << std::endl;
            }
        }
    } else {
        for(int i = 0; i < vertices.size(); i += faceSize) {
            int indices[] = {i, i + 1, i + 2};
            if(_useBinary) {
                os.write((char*) &faceSize, sizeof(unsigned char));
                for(int j = 0; j < faceSize; j++) {
                    os.write((char*) &indices[j], sizeof(uint16_t));
                }
            } else {
                os << (int) faceSize << " " << indices[0] << " " << indices[1] << " " << indices[2] << std::endl;
            }
        }
    }
    
    os.close();
}

int main(int argc, char * argv[]){
    
    //***************************************
    // decode parameters
    //***************************************
    
    // Simplification options
    float cell_size = 0.5;
    // Remove outliears
    int rm_nb_neighbors = 24;
    float rm_percentage = 0.0;
    // Normal estimation options
    int nb_neighbors = 100;
    // Poisson options
    FT sm_angle = 20.0; // Min triangle angle (degrees).
    FT sm_radius = 100; // Max triangle size w.r.t. point set average spacing.
    FT sm_distance = 0.25; // Approximation error w.r.t. point set average spacing.
    std::string solver_name = "";//"eigen"; // Sparse linear solver name.
    double approximation_ratio = 0.02;
    double average_spacing_ratio = 5;
    
    if (argc-1 < 2){
        std::cerr << "Usage: " << argv[0] << " file_in.xyz file_out.off [options]\n";
        std::cerr << "PointCloud simplification options:\n";
        std::cerr << "  -cell_size <float>          Size of the cell for the simplification (default="<<cell_size<<")\n";
        std::cerr << "\n";
        std::cerr << "PointCloud outliers removal:\n";
        std::cerr << "  -rm_nb_neighbors <float>    Nearby Neighbors for Outliers Removal (default="<<rm_nb_neighbors<<")\n";
        std::cerr << "  -rm_percentage <float>      Porcentage of outliers to remove (default="<<rm_percentage<<")\n";
        std::cerr << "\n";
        std::cerr << "PointCloud normal estimation:\n";
        std::cerr << "  -nb_neighbors <float>       Nearby Neighbors for Normal Estimation (default="<<nb_neighbors<<")\n";
        std::cerr << "\n";
        std::cerr << "Surface poisson reconstruction:\n";
        std::cerr << "  -solver <string>            Linear solver name (default="<<solver_name<<")\n";
        std::cerr << "  -sm_angle <float>           Min triangle angle (default="<< sm_angle <<" degrees)\n";
        std::cerr << "  -sm_radius <float>          Max triangle size w.r.t. point set average spacing (default="<<sm_radius<<")\n";
        std::cerr << "  -sm_distance <float>        Approximation error w.r.t. point set average spacing (default="<<sm_distance<<")\n";
        std::cerr << "  -approx <float>             Aproximation ratio (default="<<approximation_ratio<<")\n";
        std::cerr << "  -ratio <float>              Average spacing ratio (default="<<average_spacing_ratio<<")\n";
        
        return EXIT_FAILURE;
    }

    // decode parameters
    std::string input_filename  = argv[1];
    std::string output_filename = argv[2];
    for (int i=3; i+1<argc ; ++i){
        if (std::string(argv[i])=="-cell_size")
            cell_size = atof(argv[++i]);
        else if (std::string(argv[i])=="-rm_nb_neighbors")
            rm_nb_neighbors = atof(argv[++i]);
        else if (std::string(argv[i])=="-rm_nb_neighbors")
            rm_percentage = atof(argv[++i]);
        else if (std::string(argv[i])=="-nb_neighbors")
            nb_neighbors = atof(argv[++i]);
        else if (std::string(argv[i])=="-sm_radius")
            sm_angle = atof(argv[++i]);
        else if (std::string(argv[i])=="-sm_radius")
            sm_radius = atof(argv[++i]);
        else if (std::string(argv[i])=="-sm_distance")
            sm_distance = atof(argv[++i]);
        else if (std::string(argv[i])=="-solver")
            solver_name = argv[++i];
        else if (std::string(argv[i])=="-approx")
            approximation_ratio = atof(argv[++i]);
        else if (std::string(argv[i])=="-ratio")
            average_spacing_ratio = atof(argv[++i]);
        else {
            std::cerr << "Error: invalid option " << argv[i] << "\n";
            return EXIT_FAILURE;
        }
    }
	
    CGAL::Timer task_timer;
    task_timer.start();
    
    //***************************************
    // Loads mesh/point set
    //***************************************
    
    PointList points;
    std::ifstream stream(input_filename, std::ifstream::in);
    CGAL::read_xyz_points(stream, back_inserter(points));
    
    // Prints status
    int nb_points = points.size();
    std::cerr << "Reads file " << input_filename << ": " << nb_points << " points, " << task_timer.time() << " seconds" << std::endl;
    task_timer.reset();

    //***************************************
    // Simplify Point Cloud
    //***************************************
    //
    if(cell_size!=0.0){
        std::cout << "Simpliy...";
        simplifyCloud(points, cell_size);
        std::cout << points.size() << " points," << task_timer.time() << " seconds" << std::endl;
        task_timer.reset();
    }
    
    //***************************************
    // Remove outliers from Point Cloud
    //***************************************
    //
    if(rm_percentage!=0.0 && rm_nb_neighbors != 0.0){
        std::cout << "Removing outliers...";
        removeOutliers(points, rm_percentage,rm_nb_neighbors);
        std::cout << points.size() << " points," << task_timer.time() << " seconds" << std::endl;
        task_timer.reset();
    }

    //***************************************
    // Compute Normals normals on the Point Cloud
    //***************************************
    //
    PointVectorList point_vectors;
    std::cout << "Normal estimation...";
    estimateNormals(points, point_vectors, nb_neighbors);
    std::cout << task_timer.time() << " seconds" << std::endl;
    task_timer.reset();
    
    std::cout << "Normal orientation...";
    orientNormals(point_vectors, nb_neighbors, true);
    std::cout << task_timer.time() << " seconds" << std::endl;
    task_timer.reset();

    //***************************************
    // Surface Reconstruction
    //***************************************
    //
    
    PointWNList pointsWN;
    convert(point_vectors, pointsWN);
    
    // Reads the point set file in points[].
    // Note: read_xyz_points_and_normals() requires an iterator over points
    // + property maps to access each point's position and normal.
    // The position property map can be omitted here as we use iterators over Point elements.
    
    CGAL::Timer reconstruction_timer; reconstruction_timer.start();
    Counter counter(std::distance(pointsWN.begin(), pointsWN.end()));
    InsertVisitor visitor(counter) ;
    
    // Creates implicit function from the read points using the default solver.
    // Note: this method requires an iterator over points
    // + property maps to access each point's position and normal.
    // The position property map can be omitted here as we use iterators over Point_3 elements.
    Poisson_reconstruction_function function(pointsWN.begin(), pointsWN.end(),
                                             CGAL::make_identity_property_map(PointWNList::value_type()),
                                             CGAL::make_normal_of_point_with_normal_pmap(PointWNList::value_type()),
                                             visitor);
    
    #ifdef CGAL_EIGEN3_ENABLED
    // Computes the Poisson indicator function f()
    // at each vertex of the triangulation.
    if (solver_name == "eigen"){
        std::cerr << "Use Eigen 3\n";
        CGAL::Eigen_solver_traits<Eigen::ConjugateGradient<CGAL::Eigen_sparse_symmetric_matrix<double>::EigenType> > solver;
        
        if ( !function.compute_implicit_function(solver, visitor,
                                                 approximation_ratio,
                                                 average_spacing_ratio) ){
            std::cerr << "Error: cannot compute implicit function" << std::endl;
            return EXIT_FAILURE;
        }
    } else {
        if ( !function.compute_implicit_function() ){
            std::cout << "Error: cannot compute implicit function" << std::endl;
            return EXIT_FAILURE;
        }
    }
    #else
    if ( !function.compute_implicit_function() ){
        std::cout << "Error: cannot compute implicit function" << std::endl;
        return EXIT_FAILURE;
    }
    #endif
    
    //***************************************
    // Surface mesh generation
    //***************************************
    
    // Computes average spacing
    FT average_spacing = CGAL::compute_average_spacing(pointsWN.begin(), pointsWN.end(), 6 /* knn = 1 ring */);
    
    // Gets one point inside the implicit surface
    Point inner_point = function.get_inner_point();
    FT inner_point_value = function(inner_point);
    if(inner_point_value >= 0.0){
        std::cerr << "Error: unable to seed (" << inner_point_value << " at inner_point)" << std::endl;
        return EXIT_FAILURE;
    }
    
    // Gets implicit function's radius
    Sphere bsphere = function.bounding_sphere();
    FT radius = std::sqrt(bsphere.squared_radius());
    
    // Defines the implicit surface: requires defining a
    // conservative bounding sphere centered at inner point.
    FT sm_sphere_radius = 5.0 * radius;
    FT sm_dichotomy_error = sm_distance*average_spacing/1000.0; // Dichotomy error must be << sm_distance
    Surface surface(function,
                      Sphere(inner_point,sm_sphere_radius*sm_sphere_radius),
                      sm_dichotomy_error/sm_sphere_radius);
    
    // Defines surface mesh generation criteria
    CGAL::Surface_mesh_default_criteria_3<STr> criteria(sm_angle,  // Min triangle angle (degrees)
                                                        sm_radius*average_spacing,  // Max triangle size
                                                        sm_distance*average_spacing); // Approximation error
    
    CGAL_TRACE_STREAM << "  make_surface_mesh(sphere center=("<<inner_point << "),\n"
    << "                    sphere radius="<<sm_sphere_radius<<",\n"
    << "                    angle="<<sm_angle << " degrees,\n"
    << "                    triangle size="<<sm_radius<<" * average spacing="<<sm_radius*average_spacing<<",\n"
    << "                    distance="<<sm_distance<<" * average spacing="<<sm_distance*average_spacing<<",\n"
    << "                    dichotomy error=distance/"<<sm_distance*average_spacing/sm_dichotomy_error<<",\n"
    << "                    Manifold_with_boundary_tag)\n";
    
    // Generates surface mesh with manifold option
    STr tr; // 3D Delaunay triangulation for surface mesh generation
    C2t3 c2t3(tr); // 2D complex in 3D Delaunay triangulation
    CGAL::make_surface_mesh(c2t3,                                 // reconstructed mesh
                            surface,                              // implicit surface
                            criteria,                             // meshing criteria
                            CGAL::Manifold_with_boundary_tag());  // require manifold mesh
    
    // Prints status
    std::cerr << "Surface meshing: " << task_timer.time() << " seconds, "
    << tr.number_of_vertices() << " output vertices"
    << std::endl;
    task_timer.reset();
    
    if(tr.number_of_vertices() == 0)
        return EXIT_FAILURE;
    
    // Converts to polyhedron
    Polyhedron output_mesh;
    CGAL::output_surface_facets_to_polyhedron(c2t3, output_mesh);
    // Prints total reconstruction duration
    std::cerr << "Total reconstruction (implicit function + meshing): " << reconstruction_timer.time() << " seconds\n";
    
    std::cerr << "Write file " << output_filename << std::endl << std::endl;
    
    std::string extension = output_filename.substr(output_filename.find_last_of('.'));
    
    if(extension == ".off" || extension == ".OFF"){
        std::ofstream out(output_filename);
        out << output_mesh;
    } else {
        savePly(output_mesh, output_filename);
    }

    return EXIT_SUCCESS;
}
