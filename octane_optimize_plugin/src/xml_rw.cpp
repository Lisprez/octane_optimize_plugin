#include "xml_rw.h"
#include "easylogging++.h"

xml_rw::XMLHandler::XMLHandler()
{
	LOG(INFO) << "Construct XMLHandler" << std::endl;
	doc = new pugi::xml_document();
}

xml_rw::XMLHandler& xml_rw::XMLHandler::Get()
{
	static XMLHandler xml_handler{};
	return xml_handler;
}

bool xml_rw::XMLHandler::SaveFile(const std::string& file_name) const
{
	LOG(INFO) << "Save xml content to disk file." << std::endl;
	return doc->save_file(file_name.c_str());
}

xml_rw::XMLHandler::~XMLHandler()
{
	LOG(INFO) << "Destruct XMLHandler" << std::endl;
	delete doc;
}

void xml_rw::XMLHandler::Init()
{
	auto declarationNode = doc->append_child(pugi::node_declaration);
	declarationNode.append_attribute("version") = "1.0";
	declarationNode.append_attribute("encoding") = "utf-8";
	auto file_root = doc->append_child("OCS2");
	file_root.append_attribute("version") = "2250000";
}

std::string xml_rw::XMLHandler::InsertNode(const std::string& father_node_path,
	const std::string& node_name,
	const std::pair<std::string, std::string>* attr_key_value,
	const std::map<std::string, std::string>& attributes,
	const std::string& pcdata) const
{
	pugi::xml_node maybe_target_node{};

	if (!attr_key_value)
	{
		pugi::xpath_node xnode = doc->select_single_node(father_node_path.c_str());
		if (!xnode)
		{
			return father_node_path;
		}
		maybe_target_node = xnode.node();
	}
	else
	{
		auto nodes = doc->select_nodes(father_node_path.c_str());
		LOG(INFO) << "size is: " << nodes.size();
		if (nodes.empty())
		{
			return father_node_path;
		}

		for (auto& node : nodes)
		{
			if (node.node().attribute(attr_key_value->first.c_str()).value() == attr_key_value->second)
			{
				LOG(INFO) << "Found the target node!";
				maybe_target_node = node.node();
				break;
			}
		}
		if (maybe_target_node.empty())
		{
			LOG(INFO) << "Not found the target node!";
			return father_node_path;
		}
	}

	auto sub_root_node = maybe_target_node.append_child(node_name.c_str());
	if (sub_root_node) 
	{
		if (!attributes.empty())
		{
			for (auto& pair : attributes)
			{
				sub_root_node.append_attribute(pair.first.c_str()) = pair.second.c_str();
			}
		}
		
		if (!pcdata.empty())
		{
			sub_root_node.append_child(pugi::node_pcdata).set_value(pcdata.c_str());
		}

		return father_node_path + "/" + node_name;
	}
	else
	{
		return father_node_path;
	}
}

std::string xml_rw::XMLHandler::InsertRootNode(const std::string& father_node_path,
	const std::string& node_name, const std::pair<std::string,
	std::string>* diff_pair, const std::map<std::string, std::string>& attributes) const
{
	return InsertNode(father_node_path, node_name, diff_pair, attributes, "");
}

std::string xml_rw::XMLHandler::InsertPin(const std::string& father_node_path,
	const std::pair<std::string, std::string>* diff_pair,
	const std::map<std::string, std::string>& attributes) const
{
	return InsertNode(father_node_path, "pin", diff_pair, attributes, "");
}

std::string xml_rw::XMLHandler::InsertAttributeNode(const std::string& father_node_path,
	const std::pair<std::string, std::string>* diff_pair,
	const std::map<std::string, std::string>& attributes,
	const std::string& pcdata) const
{
	return InsertNode(father_node_path, "attr", diff_pair, attributes, pcdata);
}
