const fs = require('fs');
const path = require('path');
const readline = require('readline');

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

// Hàm để đọc cấu trúc thư mục
function readDirectoryStructure(dirPath, maxDepth = 3, currentDepth = 0) {
  try {
    // Kiểm tra xem đường dẫn có tồn tại không
    if (!fs.existsSync(dirPath)) {
      return {
        name: path.basename(dirPath),
        type: 'error',
        path: dirPath,
        error: 'Đường dẫn không tồn tại'
      };
    }

    // Kiểm tra xem đường dẫn có phải là thư mục không
    const stats = fs.statSync(dirPath);
    if (!stats.isDirectory()) {
      return {
        name: path.basename(dirPath),
        type: 'file',
        path: dirPath,
        size: stats.size
      };
    }

    // Đọc nội dung thư mục
    const structure = {
      name: path.basename(dirPath) || path.normalize(dirPath),
      type: 'directory',
      path: dirPath,
      children: []
    };

    // Nếu đã đạt đến độ sâu tối đa, không đọc tiếp
    if (currentDepth >= maxDepth) {
      return structure;
    }

    // Lấy danh sách các mục trong thư mục
    const items = fs.readdirSync(dirPath);

    // Duyệt qua từng mục
    for (const item of items) {
      const itemPath = path.join(dirPath, item);
      
      try {
        const itemStats = fs.statSync(itemPath);
        
        if (itemStats.isDirectory()) {
          // Nếu là thư mục, đệ quy để đọc cấu trúc bên trong
          const subDir = readDirectoryStructure(itemPath, maxDepth, currentDepth + 1);
          structure.children.push(subDir);
        } else {
          // Nếu là file, thêm vào danh sách
          structure.children.push({
            name: item,
            type: 'file',
            path: itemPath,
            size: itemStats.size
          });
        }
      } catch (err) {
        // Xử lý lỗi khi không thể đọc một mục cụ thể
        structure.children.push({
          name: item,
          type: 'error',
          path: itemPath,
          error: err.message
        });
      }
    }

    // Sắp xếp: thư mục trước, file sau, và theo thứ tự bảng chữ cái
    structure.children.sort((a, b) => {
      if (a.type === 'directory' && b.type !== 'directory') return -1;
      if (a.type !== 'directory' && b.type === 'directory') return 1;
      return a.name.localeCompare(b.name);
    });

    return structure;
  } catch (err) {
    return {
      name: path.basename(dirPath) || path.normalize(dirPath),
      type: 'error',
      path: dirPath,
      error: err.message
    };
  }
}

// Hàm hiển thị cấu trúc thư mục dưới dạng cây
function displayDirectoryTree(structure, prefix = '') {
  if (!structure) return;

  const isLast = true;
  const connector = isLast ? '└── ' : '├── ';
  const childPrefix = isLast ? '    ' : '│   ';
  
  // Hiển thị mục hiện tại
  if (structure.type === 'directory') {
    console.log(`${prefix}${connector}📁 ${structure.name}`);
  } else if (structure.type === 'file') {
    console.log(`${prefix}${connector}📄 ${structure.name}`);
  } else {
    console.log(`${prefix}${connector}❌ ${structure.name} (${structure.error})`);
  }

  // Hiển thị các mục con
  if (structure.children && structure.children.length > 0) {
    for (let i = 0; i < structure.children.length; i++) {
      const child = structure.children[i];
      const isLastChild = i === structure.children.length - 1;
      const childConnector = isLastChild ? '└── ' : '├── ';
      const nextChildPrefix = isLastChild ? '    ' : '│   ';
      
      if (child.type === 'directory') {
        console.log(`${prefix}${childPrefix}${childConnector}📁 ${child.name}`);
        
        // Hiển thị các mục con của thư mục con
        if (child.children && child.children.length > 0) {
          for (let j = 0; j < child.children.length; j++) {
            const grandchild = child.children[j];
            const isLastGrandchild = j === child.children.length - 1;
            const grandchildConnector = isLastGrandchild ? '└── ' : '├── ';
            
            if (grandchild.type === 'directory') {
              console.log(`${prefix}${childPrefix}${nextChildPrefix}${grandchildConnector}📁 ${grandchild.name}`);
            } else if (grandchild.type === 'file') {
              console.log(`${prefix}${childPrefix}${nextChildPrefix}${grandchildConnector}📄 ${grandchild.name}`);
            } else {
              console.log(`${prefix}${childPrefix}${nextChildPrefix}${grandchildConnector}❌ ${grandchild.name} (${grandchild.error})`);
            }
          }
        }
      } else if (child.type === 'file') {
        console.log(`${prefix}${childPrefix}${childConnector}📄 ${child.name}`);
      } else {
        console.log(`${prefix}${childPrefix}${childConnector}❌ ${child.name} (${child.error})`);
      }
    }
  }
}

// Hàm chính để thực thi chương trình
function main() {
  rl.question('Nhập đường dẫn thư mục cần duyệt: ', (dirPath) => {
    rl.question('Nhập độ sâu tối đa (mặc định là 3): ', (depthStr) => {
      const maxDepth = parseInt(depthStr, 10) || 3;
      
      console.log(`\nĐang đọc cấu trúc thư mục: ${dirPath} với độ sâu ${maxDepth}...\n`);
      
      const structure = readDirectoryStructure(dirPath, maxDepth);
      console.log('Cấu trúc thư mục:');
      displayDirectoryTree(structure);
      
      rl.close();
    });
  });
}

// Khởi chạy chương trình
main();