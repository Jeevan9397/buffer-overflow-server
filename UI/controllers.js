function getCookie(name) {
  const match = document.cookie.match(new RegExp('(^| )' + name + '=([^;]+)'));
  return match ? decodeURIComponent(match[2]) : '';
}

angular.module('bfApp')
.controller('LoginCtrl', function($scope, $http, $location, $window) {
  $scope.user = { user: '', pass: '', showPass: false };
  $scope.error = '';
  $scope.success = '';
  $scope.showRegister = false;
  $scope.showForgot = false;
  $scope.formErrors = {};

  $scope.togglePass = function() {
    $scope.user.showPass = !$scope.user.showPass;
  };

  // Switch mode between login/register/forgot
  $scope.switch = function(mode, $event) {
    if ($event) $event.preventDefault(); // Prevent default link behavior
    $scope.error = '';
    $scope.success = '';
    $scope.formErrors = {};
    $scope.user = { user: '', pass: '', showPass: false };
    $scope.showRegister = (mode == 'register');
    $scope.showForgot = (mode == 'forgot');
  };

  // Validate fields for all modes
  function validateForm() {
    $scope.formErrors = {};
    if (!$scope.user.user || $scope.user.user.length < 3) {
      $scope.formErrors.user = "Username must be at least 3 characters";
    }
    if (!$scope.user.pass || $scope.user.pass.length < 5) {
      $scope.formErrors.pass = "Password must be at least 5 characters";
    }
    return Object.keys($scope.formErrors).length === 0;
  }

  // Main form submit for all three modes
  $scope.submit = function() {
    $scope.error = '';
    $scope.success = '';

    // Validate input
    if (!validateForm()) return;

    if ($scope.showRegister) {
      // Registration API call
      $http.post('/api/register', {
        user: $scope.user.user,
        pass: $scope.user.pass
      }).then(function(response) {
        if (response.data.status === 'ok' || response.status === 201) {
          $scope.success = "Registration successful! Please login.";
          $scope.switch('login');
        } else {
          $scope.error = "Registration failed. Try a different username.";
        }
      }, function(err) {
        if (err.status === 409) {
          $scope.error = "Username already exists. Try another.";
        } else if (typeof err.data === "string") {
          $scope.error = err.data;
        } else {
          $scope.error = "Registration error.";
        }
      });
    }
    else if ($scope.showForgot) {
      // Forgot password API call
      $http.post('/api/forgot', {
        user: $scope.user.user,
        pass: $scope.user.pass
      }).then(function(response) {
        if (response.data.status === 'ok') {
          $scope.success = "Password reset! You can now login.";
          $scope.switch('login');
        } else {
          $scope.error = "Reset failed. Try again.";
        }
      }, function(err) {
        if (err.status === 404) {
          $scope.error = "Username not found.";
        } else if (typeof err.data === "string") {
          $scope.error = err.data;
        } else {
          $scope.error = "Reset error.";
        }
      });
    }
    else {
      // Login API call
      $http.post('/api/login', {
        user: $scope.user.user,
        pass: $scope.user.pass
      }).then(function(response) {
        if (response.data.status === 'ok') {
          // Redirect to dashboard (SPA route)
          $scope.success = response.data.message || "Logged in successfully";
          $location.path('/dashboard');
        } else {
          $scope.error = "Login failed. Please try again.";
        }
      }, function() {
        $scope.error = "Invalid username or password.";
      });
    }
  };
})

.controller('DashboardCtrl', function($scope, $http, $location) {
  $scope.username = getCookie('user');
  $scope.records = [];
  $scope.newRecord = {};
  $scope.showAddModal = false;
  $scope.editing = false;
  $scope.selectedRecord = null;
  $scope.selectedRecords = [];
  $scope.lastSelected = null;

  $scope.triggerImport = function () {
  document.getElementById('importFile').click();
};

$scope.import = function () {
  document.getElementById('importFile').click();
};

$scope.importFileChanged = function (input) {
  const file = input.files[0];
  if (!file) return;

  const formData = new FormData();
  formData.append("file", file);

  $http.post("/api/import", formData, {
    transformRequest: angular.identity,
    headers: { "Content-Type": undefined }
  }).then(function (res) {
    alert("Import successful!");
    refreshRecords();
  }, function (err) {
    alert("Import failed.");
    console.error(err);
  });

  // Clear the input so user can re-import same file later
  input.value = '';
};



  // Selection logic: ctrl-click, shift-click, or single
  $scope.selectRecord = function(record, $event) {
    const index = $scope.selectedRecords.indexOf(record);

    if ($event.ctrlKey || $event.metaKey) {
      if (index === -1) {
        $scope.selectedRecords.push(record);
      } else {
        $scope.selectedRecords.splice(index, 1);
      }
    } else if ($event.shiftKey && $scope.lastSelected) {
      const last = $scope.records.indexOf($scope.lastSelected);
      const current = $scope.records.indexOf(record);
      const [start, end] = [Math.min(last, current), Math.max(last, current)];
      $scope.selectedRecords = $scope.records.slice(start, end + 1);
    } else {
      if ($scope.selectedRecords.length === 1 && index !== -1) {
        $scope.selectedRecords = [];
      } else {
        $scope.selectedRecords = [record];
      }
    }

    $scope.lastSelected = record;
    $scope.selectedRecord = $scope.selectedRecords[0] || null;
  };

  function formatDate(isoDate) {
    const d = new Date(isoDate);
    if (!isNaN(d)) {
      const dd = String(d.getDate()).padStart(2, '0');
      const mm = String(d.getMonth() + 1).padStart(2, '0');
      const yyyy = d.getFullYear();
      return `${dd}-${mm}-${yyyy}`;
    }
    return isoDate;
  }

  function parseDate(dobStr) {
  if (!dobStr) return '';

  if (dobStr instanceof Date && !isNaN(dobStr)) {
    const yyyy = dobStr.getFullYear();
    const mm = String(dobStr.getMonth() + 1).padStart(2, '0');
    const dd = String(dobStr.getDate()).padStart(2, '0');
    return `${yyyy}-${mm}-${dd}`;
  }

  // Replace dots with dashes to handle "10.10.1997"
  dobStr = dobStr.replace(/\./g, '-');

  const parts = dobStr.split('-');
  if (parts.length === 3 && parts[2].length === 4) {
    return `${parts[2]}-${parts[1].padStart(2, '0')}-${parts[0].padStart(2, '0')}`;
  }

  return dobStr;
}


  function formatCustomers(customers) {
    return customers.map(c => ({
      id: c.id,
      name: c.name,
      father: c.father,
      nationality: c.nationality,
      dob: c.dob,
      dobFormatted: formatDate(c.dob),
      birthplace: c.birthplace,
      address: c.address,
      mobile: c.mobile,
      email: c.email
    }));
  }

  function refreshRecords() {
    $http.get('/api/customers').then(function(resp) {
      $scope.records = formatCustomers(resp.data.customers);
      $scope.selectedRecords = [];
      $scope.selectedRecord = null;
    });
  }

  refreshRecords();

  $scope.add = function() {
    $scope.newRecord = {};
    $scope.editing = false;
    $scope.showAddModal = true;
  };

  $scope.cancelAdd = function() {
    $scope.showAddModal = false;
    $scope.editing = false;
    $scope.newRecord = {};
  };

  $scope.triggerEdit = function() {
    if ($scope.selectedRecords.length === 0) {
      alert("Please select a record to edit.");
      return;
    }
    $scope.editRecord($scope.selectedRecords[0]);
  };

  $scope.editRecord = function(record) {
    $scope.newRecord = angular.copy(record);
    $scope.newRecord.dob = new Date(record.dob);
    $scope.editing = true;
    $scope.showAddModal = true;
  };

  $scope.saveRecord = function() {
    const payload = {
      name: $scope.newRecord.name,
      father: $scope.newRecord.father,
      nationality: $scope.newRecord.nationality,
      dob: parseDate($scope.newRecord.dob),
      birthplace: $scope.newRecord.birthplace,
      address: $scope.newRecord.address,
      mobile: $scope.newRecord.mobile,
      email: $scope.newRecord.email
    };

    const endpoint = $scope.editing
      ? `/api/customers/${$scope.newRecord.id}`
      : '/api/customers';

    const method = $scope.editing ? $http.put : $http.post;

    method(endpoint, payload).then(function() {
      refreshRecords();
      $scope.showAddModal = false;
      $scope.editing = false;
      $scope.newRecord = {};
    }, function() {
      alert("Error saving record.");
    });
  };

  $scope.export = function () {
    let ids = [];

    if ($scope.selectedRecords && $scope.selectedRecords.length > 0) {
      ids = $scope.selectedRecords.map(r => r.id);
    }

    let url = '/api/export';
    if (ids.length > 0 && ids.length !== $scope.records.length) {
      url += '?ids=' + ids.join(',');
    }

    const a = document.createElement('a');
    a.href = url;
    a.download = 'customers.csv';
    a.click();
  };

  $scope.delete = function() {
    if ($scope.selectedRecords.length === 0) {
      alert("Please select at least one record to delete.");
      return;
    }

    if (!confirm(`Delete ${$scope.selectedRecords.length} selected record(s)?`)) return;

    const toDelete = [...$scope.selectedRecords];

    let deletedCount = 0;
    toDelete.forEach((rec, idx) => {
      $http.delete(`/api/customers/${rec.id}`).then(() => {
        deletedCount++;
        if (deletedCount === toDelete.length) {
          refreshRecords();
        }
      }, () => {
        alert("Error deleting record with ID: " + rec.id);
      });
    });
  };

  $scope.logout = function() {
    $location.path('/login');
  };
});
