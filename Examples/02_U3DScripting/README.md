# 03_UIEvent

## 代码

```lua
function start()
	print("lua start...")
	--lua 函数的转换
	self:GetComponent("Button").onClick:AddListener(function()
		print("clicked, you input is '" ..input:GetComponent("InputField").text .."'")
	end)
end
```