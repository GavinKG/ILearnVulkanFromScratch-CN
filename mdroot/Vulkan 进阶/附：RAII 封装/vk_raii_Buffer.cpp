class Buffer
{
public:

	using CType = VkBuffer;

	static constexpr vk::ObjectType objectType = vk::ObjectType::eBuffer;
	static constexpr vk::DebugReportObjectTypeEXT debugReportObjectType = vk::DebugReportObjectTypeEXT::eBuffer;

    // 创建 Buffer 的构造函数
	Buffer(vk::raii::Device const& device,
		vk::BufferCreateInfo const& createInfo,
		vk::Optional<const vk::AllocationCallbacks> allocator = nullptr)
		: m_device(*device)
		, m_allocator(static_cast<const vk::AllocationCallbacks*>(allocator))
		, m_dispatcher(device.getDispatcher())
	{
		vk::Result result = static_cast<vk::Result>(
			device.getDispatcher()->vkCreateBuffer(static_cast<VkDevice>(*device),
				reinterpret_cast<const VkBufferCreateInfo*>(&createInfo),
				reinterpret_cast<const VkAllocationCallbacks*>(m_allocator),
				reinterpret_cast<VkBuffer*>(&m_buffer)));
		if (result != vk::Result::eSuccess)
		{
			throwResultException(result, "vkCreateBuffer");
		}
	}

    // 接受已有 VkBuffer Handle 的构造函数
	Buffer(vk::raii::Device const& device,
		VkBuffer buffer,
		vk::Optional<const vk::AllocationCallbacks> allocator = nullptr)
		: m_device(*device)
		, m_buffer(buffer)
		, m_allocator(static_cast<const vk::AllocationCallbacks*>(allocator))
		, m_dispatcher(device.getDispatcher())
	{
	}

    // nullptr 构造函数。
    // 如果不想立刻初始化资源时可以用 vk::raii::Buffer buffer { nullptr } 的方法初始化，
    // 后期使用移动构造函数 / 移动赋值运算符初始化该资源。因此可以在栈上实现延迟资源初始化。
    // 可以看出，该类实现了 pointer-like。
	Buffer(std::nullptr_t) {}

    // 析构函数，负责销毁资源。
	~Buffer()
	{
		if (m_buffer)
		{
			getDispatcher()->vkDestroyBuffer(static_cast<VkDevice>(m_device),
				static_cast<VkBuffer>(m_buffer),
				reinterpret_cast<const VkAllocationCallbacks*>(m_allocator));
		}
	}

    // 禁用默认构造函数。
	Buffer() = delete;

    // 禁用拷贝构造函数。
    // 几乎所有跟 VkHandle 打交道的 RAII Wrapper 类都必须禁用掉拷贝构造/赋值，强调资源的占有（Ownership）特性。
    // 可以看出，该类只能使用 std::unique_ptr 类型智能指针进行正确指向。
	Buffer(Buffer const&) = delete;

    // 移动构造函数，使用 std::exchange，将资源的占有（Ownership）转移。
	Buffer(Buffer&& rhs) noexcept
		: m_device(vk::raii::exchange(rhs.m_device, {}))
		, m_buffer(vk::raii::exchange(rhs.m_buffer, {}))
		, m_allocator(vk::raii::exchange(rhs.m_allocator, {}))
		, m_dispatcher(vk::raii::exchange(rhs.m_dispatcher, nullptr))
	{
	}

    // 同理，禁用拷贝赋值运算符。
	Buffer& operator=(Buffer const&) = delete;

    // 移动赋值运算符。
	Buffer& operator =(Buffer&& rhs) noexcept
	{
		if (this != &rhs)
		{
			m_device = vk::raii::exchange(rhs.m_device, {});
			m_buffer = vk::raii::exchange(rhs.m_buffer, {});
			m_allocator = vk::raii::exchange(rhs.m_allocator, {});
			m_dispatcher = vk::raii::exchange(rhs.m_dispatcher, nullptr);
		}
		return *this;
	}

    // 通过 pointer-like 设计方法，暴露 RAII 所管理的资源。
	vk::Buffer const& operator*() const noexcept
	{
		return m_buffer;
	}

    // 获取该缓冲所对应的物理设备（VkDevice）
	vk::Device getDevice() const
	{
		return m_device;
	}

	vk::raii::DeviceDispatcher const* getDispatcher() const
	{
		assert(m_dispatcher->getVkHeaderVersion() == VK_HEADER_VERSION);
		return m_dispatcher;
	}

	//=== VK_VERSION_1_0 ===

	void bindMemory(vk::DeviceMemory memory, vk::DeviceSize memoryOffset) const;

	[[nodiscard]] vk::MemoryRequirements getMemoryRequirements() const noexcept;

private:
	vk::Device m_device = {};
	vk::Buffer m_buffer = {};
	const vk::AllocationCallbacks* m_allocator = nullptr;
	vk::raii::DeviceDispatcher const* m_dispatcher = nullptr;
};