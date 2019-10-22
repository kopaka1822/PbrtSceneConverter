#include "Plymesh.h"
#include "../rply/rply.h"

struct CallbackContext {
	ei::Vec3 *p;
	ei::Vec3 *n;
	ei::Vec2 *uv;
	int *indices;
	int indexCtr;
	int face[4];
	bool error;
	int vertexCount;

	CallbackContext()
		: p(nullptr),
		n(nullptr),
		uv(nullptr),
		indices(nullptr),
		indexCtr(0),
		error(false),
		vertexCount(0) {}

	~CallbackContext() {
		delete[] p;
		delete[] n;
		delete[] uv;
		delete[] indices;
	}
};

void rply_message_callback(p_ply ply, const char *message) {
	System::info(std::string("rply: ") + message);
}

/* Callback to handle vertex data from RPly */
int rply_vertex_callback(p_ply_argument argument) {
	float **buffers;
	long index, flags;

	ply_get_argument_user_data(argument, (void**)(&buffers), &flags);
	ply_get_argument_element(argument, nullptr, &index);

	int bufferIndex = (flags & 0xF00) >> 8;
	int stride = (flags & 0x0F0) >> 4;
	int offset = flags & 0x00F;

	float *buffer = buffers[bufferIndex];
	if (buffer)
		buffer[index * stride + offset] =
		float(ply_get_argument_value(argument));

	return 1;
}

/* Callback to handle face data from RPly */
int rply_face_callback(p_ply_argument argument) {
	long length, value_index;
	ply_get_argument_property(argument, nullptr, &length, &value_index);

	if (length != 3 && length != 4) {
		System::warning("ply: ignoring faces with " + std::to_string(length) + "vertices (only triangles and quads are supported!)");
		return 1;
	}
	else if (value_index < 0) {
		return 1;
	}

	CallbackContext *context;
	ply_get_argument_user_data(argument, (void **)&context, nullptr);

	if (value_index >= 0) {
		int value = (int)ply_get_argument_value(argument);
		if (value < 0 || value >= context->vertexCount) {
			System::error("ply: Vertex reference" + std::to_string(value) +
				"is out of bounds! Valid range is [0.." + std::to_string(context->vertexCount) + "]");
			context->error = true;
		}
		context->face[value_index] = value;
	}

	if (value_index == length - 1) {
		for (int i = 0; i < 3; ++i)
			context->indices[context->indexCtr++] = context->face[i];

		if (length == 4) {
			/* This was a quad */
			context->indices[context->indexCtr++] = context->face[3];
			context->indices[context->indexCtr++] = context->face[0];
			context->indices[context->indexCtr++] = context->face[2];
		}
	}
	return 1;
}

void Plymesh::init(ParamSet& set)
{
	auto filename = set.getString("filename", "");
	if (!filename.length())
		throw PbrtMissingParameter("filename");

	filename = System::fixPath(System::getCurrentDirectory() + filename);
	auto ply = ply_open(filename.c_str(), rply_message_callback, 0, nullptr);
	if(!ply)
	{
		System::error("could not open ply file " + filename);
		return;
	}

	if(!ply_read_header(ply))
	{
		System::error("unable to read the header of PLY file " + filename);
		return;
	}

	p_ply_element element = nullptr;
	long vertexCount = 0, faceCount = 0;

	/* Inspect the structure of the PLY file */
	while ((element = ply_get_next_element(ply, element)) != nullptr) {
		const char *name;
		long nInstances;

		ply_get_element_info(element, &name, &nInstances);
		if (!strcmp(name, "vertex"))
			vertexCount = nInstances;
		else if (!strcmp(name, "face"))
			faceCount = nInstances;
	}

	if (vertexCount == 0 || faceCount == 0) {
		System::error("PLY file " + filename +  " is invalid! No face/vertex elements found!");
		return;
	}

	CallbackContext context;

	if (ply_set_read_cb(ply, "vertex", "x", rply_vertex_callback, &context,
		0x030) &&
		ply_set_read_cb(ply, "vertex", "y", rply_vertex_callback, &context,
			0x031) &&
		ply_set_read_cb(ply, "vertex", "z", rply_vertex_callback, &context,
			0x032)) {
		context.p = new ei::Vec3[vertexCount];
	}
	else {
		System::error("PLY file " + filename + ": Vertex coordinate property not found!");
		return;
	}

	if (ply_set_read_cb(ply, "vertex", "nx", rply_vertex_callback, &context,
		0x130) &&
		ply_set_read_cb(ply, "vertex", "ny", rply_vertex_callback, &context,
			0x131) &&
		ply_set_read_cb(ply, "vertex", "nz", rply_vertex_callback, &context,
			0x132))
		context.n = new ei::Vec3[vertexCount];
	
	/* There seem to be lots of different conventions regarding UV coordinate
	* names */
	if ((ply_set_read_cb(ply, "vertex", "u", rply_vertex_callback, &context,
		0x220) &&
		ply_set_read_cb(ply, "vertex", "v", rply_vertex_callback, &context,
			0x221)) ||
			(ply_set_read_cb(ply, "vertex", "s", rply_vertex_callback, &context,
				0x220) &&
				ply_set_read_cb(ply, "vertex", "t", rply_vertex_callback, &context,
					0x221)) ||
					(ply_set_read_cb(ply, "vertex", "texture_u", rply_vertex_callback,
						&context, 0x220) &&
						ply_set_read_cb(ply, "vertex", "texture_v", rply_vertex_callback,
							&context, 0x221)) ||
							(ply_set_read_cb(ply, "vertex", "texture_s", rply_vertex_callback,
								&context, 0x220) &&
								ply_set_read_cb(ply, "vertex", "texture_t", rply_vertex_callback,
									&context, 0x221)))
		context.uv = new ei::Vec2[vertexCount];

	/* Allocate enough space in case all faces are quads */
	context.indices = new int[faceCount * 6];
	context.vertexCount = vertexCount;

	ply_set_read_cb(ply, "face", "vertex_indices", rply_face_callback, &context,
		0);

	if (!ply_read(ply)) {
		System::error("unable to read the contents of PLY file " + filename);
		ply_close(ply);
		return;
	}

	ply_close(ply);
	if (context.error)
	{
		System::error("error during read of PLY file " + filename);
		return;
	}

	m_geom->m_indices.reserve(context.indexCtr);
	m_geom->m_indices.assign(context.indices, context.indices + context.indexCtr);
	
	m_geom->m_p.reserve(context.vertexCount);
	m_geom->m_p.assign(context.p, context.p + context.vertexCount);

	if(context.n)
	{
		m_geom->m_n.reserve(context.vertexCount);
		m_geom->m_n.assign(context.n, context.n + context.vertexCount);
	}
	
	bool discardDegenerateUVs = set.getBool("discarddegenerateUVs", false);
	if (discardDegenerateUVs && m_geom->m_uv.size())
	{
		std::vector<float> rawUvs;
		rawUvs.reserve(context.vertexCount * 2);
		rawUvs.assign(reinterpret_cast<float*>(context.uv), reinterpret_cast<float*>(context.uv) + 2 * context.vertexCount);
		discardDegenerateUvs(rawUvs);
		copyToVec2(m_geom->m_uv, rawUvs);
	}
	else if(context.uv)
	{
		m_geom->m_uv.reserve(context.vertexCount);
		m_geom->m_uv.assign(context.uv, context.uv + context.vertexCount);
	}

	verifyData(true);
	System::runtimeInfoSpam("parsed plymesh " + filename);
}
