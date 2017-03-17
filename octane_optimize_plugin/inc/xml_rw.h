#pragma once

#include <map>

#include "pugixml.hpp"
#include "pugiconfig.hpp"

namespace xml_rw {
	
	class XMLHandler {
	public:
		XMLHandler(const XMLHandler&) = delete;
		XMLHandler& operator =(const XMLHandler&) = delete;
		XMLHandler(const XMLHandler&&) = delete;
		XMLHandler& operator =(const XMLHandler&&) = delete;

		static XMLHandler& Get();
		~XMLHandler();
		
		bool SaveFile(const std::string& file_name) const;

		// 初始化xml文件, 并插入根结点
		void Init();
		
		// 插入根结点
		std::string InsertRootNode(const std::string& father_node_path,
			const std::string& node_name,
			const std::pair<std::string, std::string>* diff_pair,
			const std::map<std::string, std::string>& attributes) const;

		// 插入PIN结点
		std::string InsertPin(const std::string& father_node_path,
			const std::pair<std::string, std::string>* diff_pair,
			const std::map<std::string, std::string>& attributes) const;

		// 插入属性结点
		std::string InsertAttributeNode(const std::string& father_node_path,
			const std::pair<std::string, std::string>* diff_pair,
			const std::map<std::string, std::string>& attributes,
			const std::string& pcdata) const;

		/**
		 * 用于向XML树中插入一个新的结点
		 * @father_node_path: 表示在这个节点下插入新的节点
		 * @node_name: 新结点的名称
		 * @attri_key_value: 一个键值对用于从名字相同的节点出唯一确定出父结点
		 * @attribute: 插入结点的属性集
		 * 成功返回插入的新结点的路径, 失败返回父结点的路径
		 */
		std::string InsertNode(const std::string& father_node_path, 
			const std::string& node_name, 
			const std::pair<std::string, std::string>* attr_key_value,
			const std::map<std::string, std::string>& attributes,
			const std::string& pcdata) const;
	private:

		XMLHandler();
		pugi::xml_document* doc = nullptr;
	};

}
