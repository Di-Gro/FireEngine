#pragma once

class SafeDestroy {
protected:
	int timeToDestroy = 1;

private:
	bool m_onPreDestroy = false;
	void* m_pointer = nullptr;

public:
	bool IsDestroyed() { return m_pointer == nullptr; }

	virtual void Destroy() = 0;

protected:

	template<typename T>
	T* pointerForDestroy() { return (T*)m_pointer; }

	void pointerForDestroy(void* pointer) {
		assert(pointer != nullptr);
		m_pointer = pointer;
	}

	bool BeginDestroy() { 
		if (!IsDestroyed() && !m_onPreDestroy) {
			m_onPreDestroy = true;
			return true;
		}
		return false;
	}

	void EndDestroy() {
		m_pointer = nullptr;
	}
};